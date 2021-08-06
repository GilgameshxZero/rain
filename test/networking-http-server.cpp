// Tests Networking::Http::Server.
#include <rain/time/time.hpp>

#include <rain/networking/http/client.hpp>
#include <rain/networking/http/server.hpp>

#include <cassert>
#include <iostream>

using namespace Rain::Literal;
using namespace Rain::Networking::Http;

class AssertWorker final : public WorkerInterface<Socket> {
	public:
	using Interface::WorkerInterface;

	private:
	// Extend preprocess chain.
	virtual void streamInImpl(Tag<Interface>, Request &req) final override {
		std::cout << req.method << " " << req.target << " HTTP/" << req.version
							<< std::endl;
	}

	// Extend match chain.
	PreResponse getSimple(Request &req, std::smatch const &) {
		std::stringstream bodyStream;
		bodyStream << req.body;

		assert(req.version == Version::_1_1);
		assert(req.headers.size() == 1);
		assert(req.headers.contentLength() == 0);
		assert(bodyStream.str().empty());
		return {StatusCode::OK};
	}
	PreResponse get09(Request &req, std::smatch const &match) {
		std::stringstream bodyStream;
		bodyStream << req.body;

		assert(req.version == Version::_0_9);
		assert(req.headers.empty());
		assert(bodyStream.str().empty());

		// Return the match as body.
		return {
			StatusCode::INTERNAL_SERVER_ERROR,
			{{{"Header", "No headers should be send either, but the body should send."}}},
			{match.size() > 1 ? match[1].str() : ""s},
			"No reason phrase should be sent as part of an HTTP/0.9 Response.",
			Version::_0_9};
	}
	PreResponse postApiAssert(Request &req, std::smatch const &) {
		std::stringstream bodyStream;
		bodyStream << req.body;

		// Verify Body-Copy header mirrors body.
		assert(req.headers["Body-Copy"] == bodyStream.str());

		return {StatusCode::OK, {}, {"Verified."}, "", req.version};
	}
	virtual std::optional<PreResponse> chainMatchTargetImpl(
		Tag<Interface>,
		Request &req) final override {
		std::smatch match;
		if (
			req.method == Method::GET &&
			std::regex_match(req.target, match, "/0\\.9-?(.*)"_re)) {
			return this->get09(req, match);
		} else if (
			req.method == Method::GET &&
			std::regex_match(req.target, match, "/simple"_re)) {
			return this->getSimple(req, match);
		} else if (
			req.method == Method::POST &&
			std::regex_match(req.target, match, "/api/assert"_re)) {
			return this->postApiAssert(req, match);
		}
		return {};
	}

	// Extend postprocess chain.
	virtual void streamOutImpl(Tag<Interface>, Response &res) final override {
		std::cout << "HTTP/" << res.version << " " << res.statusCode << " "
							<< res.reasonPhrase << std::endl;
	}
};
typedef ServerInterface<Socket, AssertWorker> AssertServer;

class PlayWorker final : public WorkerInterface<Socket> {
	public:
	using Interface::WorkerInterface;

	private:
	// Extend preprocess chain.
	virtual void streamInImpl(Tag<Interface>, Request &req) final override {
		std::cout << req.method << " " << req.target << " HTTP/" << req.version
							<< std::endl;
	}

	// Extend match chain.
	PreResponse getApi(Request &req, std::smatch const &) {
		std::stringstream resBody;
		resBody << "Your hostname: " << this->getTargetHost() << "\r\n"
						<< "Your request:\r\n"
						<< req.method << " " << req.target << " HTTP/" << req.version
						<< "\r\n"
						<< req.headers << "\r\n"
						<< req.body;
		return {StatusCode::OK, {}, {resBody.str()}};
	}
	virtual std::optional<PreResponse> chainMatchTargetImpl(
		Tag<Interface>,
		Request &req) final override {
		std::smatch match;
		if (
			req.method == Method::GET &&
			std::regex_match(req.target, match, "/api(/.*)?"_re)) {
			return this->getApi(req, match);
		}
		return {};
	}

	// Extend postprocess chain.
	virtual void streamOutImpl(Tag<Interface>, Response &res) final override {
		std::cout << "HTTP/" << res.version << " " << res.statusCode << " "
							<< res.reasonPhrase << std::endl;
	}
};
typedef ServerInterface<Socket, PlayWorker> PlayServer;

int main() {
	// Send some requests (and receive on Server-side), receive some responses
	// (sent from Server-side).
	{
		// Idling for over 5s will abort the connection.
		AssertServer server(
			1024,
			Rain::Networking::Specification::ProtocolFamily::INET6,
			1_zu << 10,
			1_zu << 10,
			5s,
			5s);
		server.serve();
		std::cout << "Serving on " << server.getTargetHost() << "..." << std::endl;
		Rain::Networking::Host serverHost{
			"127.0.0.1", server.getTargetHost().service};

		// Basic 1.1 GET request.
		{
			Client client;
			client.connect(serverHost);

			// Both send and << should be subject to postprocessing.
			client.send(Request{Method::GET, "/simple"});
			Response res = client.recvResponse();

			assert(res.version == Version::_1_1);
			assert(res.statusCode == StatusCode::OK);
			assert(res.reasonPhrase == "OK");
			assert(res.headers.size() == 1);
			assert(res.headers.contentLength() == 0);
			std::stringstream bodyStream;
			bodyStream << res.body;
			assert(bodyStream.str().empty());
		}

		// Basic 0.9 GET request. Server closes connection afterwards.
		{
			Client client;
			client.connect(serverHost);
			client << Request{Method::GET, "/0.9", {}, {}, Version::_0_9};
			Response res = client.recvResponse();

			assert(res.version == Version::_0_9);
			// HTTP/0.9 Response status code and reason phrase are always left
			// default.
			assert(res.statusCode == StatusCode::OK);
			assert(res.reasonPhrase == "");
			assert(res.headers.size() == 0);
			std::stringstream bodyStream;
			bodyStream << res.body;
			assert(bodyStream.str().empty());
		}

		// HTTP/0.9 request with headers and bodies. They should be ignored
		// correctly.
		{
			Client client;
			client.connect(serverHost);
			client << Request{
				Method::GET,
				"/0.9-some-string-of-chars-to-be-matched",
				{{{"Header", "This header should not appear in an HTTP/0.9 request."}}},
				{"No bodies allowed in HTTP/0.9 requests either."},
				Version::_0_9};
			Response res = client.recvResponse();

			assert(res.version == Version::_0_9);
			// HTTP/0.9 Response status code and reason phrase are always left
			// default.
			assert(res.statusCode == StatusCode::OK);
			assert(res.reasonPhrase == "");
			assert(res.headers.size() == 0);
			std::stringstream bodyStream;
			bodyStream << res.body;
			assert(bodyStream.str() == "some-string-of-chars-to-be-matched");
		}

		// Request to a 404 resource should be caught by Http implementation.
		{
			Client client;
			client.connect(serverHost);
			client << Request{
				Method::GET, "/DO_NOT_MATCH_THIS", {}, {}, Version::_1_0};
			Response res = client.recvResponse();

			assert(res.version == Version::_1_0);
			assert(res.statusCode == StatusCode::NOT_FOUND);
			assert(res.reasonPhrase == "Not Found");
			assert(res.headers.size() == 1);
			assert(res.headers.contentLength() == 0);
			std::stringstream bodyStream;
			bodyStream << res.body;
			assert(bodyStream.str() == "");
		}

		// Post a body, expect to be same as header.
		// Send a followup request with Transfer-Encoding chunked post.
		{
			std::string body = "hello world";
			Client client;
			client.connect(serverHost);

			{
				client << Request{
					Method::POST, "/api/assert", {{{"Body-Copy", body}}}, {body}};
				Response res = client.recvResponse();

				std::stringstream bodyStream;
				bodyStream << res.body;
				assert(bodyStream.str() == "Verified.");
			}

			// Followup request on the same connection with a manually-constructed
			// chunked body.
			{
				client << Request{
					Method::POST,
					"/api/assert",
					{{{"Transfer-Encoding", "chunked"}, {"Body-Copy", body}}},
					{"5\r\nhello\r\n1\r\n \r\n5\r\nworld\r\n0\r\n\r\n"}};
				Response res = client.recvResponse();

				std::stringstream bodyStream;
				bodyStream << res.body;
				assert(bodyStream.str() == "Verified.");
			}
		}

		// Idling for over 5s should be kicked by maxRecvIdleDuration.
		{
			Client client;
			client.connect(serverHost);

			std::this_thread::sleep_for(7s);
			client << Request{Method::GET, "/"};
			Response res = client.recvResponse();
			assert(!client.good());
		}
	}

	// Open a playtest server for 60s.
	{
		PlayServer server;
		server.serve();
		std::cout << "Serving on " << server.getTargetHost() << "..." << std::endl;
		std::this_thread::sleep_for(60s);
		server.close();
	}

	return 0;
}
