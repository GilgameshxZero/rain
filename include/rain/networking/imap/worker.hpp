#pragma once

#include "../req_res/worker.hpp"
#include "request.hpp"
#include "response.hpp"
#include "socket.hpp"

namespace Rain::Networking::Imap {
	class WorkerSocketSpecInterfaceInterface :
		virtual public ConnectedSocketSpecInterface,
		virtual public ReqRes::
			WorkerSocketSpecInterfaceInterface {};

	template<
		typename RequestMessageSpec,
		typename ResponseMessageSpec>
	class WorkerSocketSpecInterface :
		virtual public WorkerSocketSpecInterfaceInterface,
		virtual public ReqRes::WorkerSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec> {
		protected:
		class ResponseAction {
			public:
			std::optional<ResponseMessageSpec> response;

			// If true, closes after the ResponseMessageSpec is
			// sent, or aborts with no ResponseMessageSpec.
			bool toClose;

			// Close.
			ResponseAction() : toClose(true), response() {}

			// Send response, optionally close.
			ResponseAction(
				ResponseMessageSpec &&response,
				bool toClose = false) :
				response(
					std::forward<ResponseMessageSpec>(response)),
				toClose(toClose) {}

			// Don't send response, optionally close.
			ResponseAction(std::nullptr_t, bool toClose) :
				response(),
				toClose(toClose) {}

			ResponseAction(ResponseAction const &) = delete;
			ResponseAction &operator=(
				ResponseAction const &) = delete;
			ResponseAction(ResponseAction &&other) = delete;
			ResponseAction &operator=(ResponseAction &&) = delete;
		};

		private:
		virtual bool onRequest(
			RequestMessageSpec &req) override final {
			// TODO: WIP testing stage.
			std::cout << req << std::flush;
			if (
				dynamic_cast<Command::Capability *>(
					req.command.get()) != nullptr) {
				{
					ResponseMessageSpec res;
					res.command.reset(
						new Command::Capability("IMAP4rev1"));
					this->send(res);
					std::cout << res << std::flush;
				}
				{
					ResponseMessageSpec res;
					res.tag = req.tag;
					res.command.reset(new Command::Ok());
					this->send(res);
					std::cout << res << std::flush;
				}
			} else if (
				dynamic_cast<Command::Authenticate *>(
					req.command.get()) != nullptr) {
				{
					ResponseMessageSpec res;
					res.tag = req.tag;
					res.command.reset(new Command::Bad());
					this->send(res);
					std::cout << res << std::flush;
				}
			} else if (
				dynamic_cast<Command::Login *>(req.command.get()) !=
				nullptr) {
				{
					ResponseMessageSpec res;
					res.tag = req.tag;
					res.command.reset(new Command::Ok());
					this->send(res);
					std::cout << res << std::flush;
				}
			} else if (
				dynamic_cast<Command::List *>(req.command.get()) !=
				nullptr) {
				{
					ResponseMessageSpec res;
					res.command.reset(new Command::List(
						"(\\HasNoChildren) \"/\" \"inbox\""));
					this->send(res);
					std::cout << res << std::flush;
				}
				{
					ResponseMessageSpec res;
					res.tag = req.tag;
					res.command.reset(new Command::Ok());
					this->send(res);
					std::cout << res << std::flush;
				}
			} else if (
				dynamic_cast<Command::Select *>(
					req.command.get()) != nullptr) {
				{
					std::string res{"* FLAGS (\\Deleted \\Seen)\r\n* "
													"1 EXISTS\r\n* 0 RECENT\r\n"};
					this->send(res);
					std::cout << res << std::flush;
				}
				{
					ResponseMessageSpec res;
					res.tag = req.tag;
					res.command.reset(new Command::Ok());
					this->send(res);
					std::cout << res << std::flush;
				}
			}
			return false;
		}
		virtual bool onInitialResponse() override {
			ResponseMessageSpec res;
			res.command.reset(new Command::Ok("IMAP4rev1"));
			this->send(res);
			std::cout << res << std::flush;
			return false;
		}
	};

	template<
		typename RequestMessageSpec,
		typename ResponseMessageSpec,
		typename Socket>
	class WorkerSocketSpec :
		public Socket,
		virtual public WorkerSocketSpecInterface<
			RequestMessageSpec,
			ResponseMessageSpec> {
		using Socket::Socket;
	};

	// Shorthand, but importantly names *SocketSpec, which is
	// consistent across each layer, and overwritten by the
	// next protocol layer, useful for deducing types on the
	// previous layer (e.g. for TLS).
	template<
		typename RequestMessageSpec = Imap::Request,
		typename ResponseMessageSpec = Imap::Response,
		typename SocketFamilyInterface = Ipv4FamilyInterface,
		template<typename> class... SocketOptions>
	class Worker :
		public WorkerSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<ReqRes::Worker<
					RequestMessageSpec,
					ResponseMessageSpec,
					SocketFamilyInterface,
					SocketOptions...>>>>> {
		public:
		using WorkerSocketSpec = WorkerSocketSpec<
			RequestMessageSpec,
			ResponseMessageSpec,
			ConnectedSocketSpec<
				NamedSocketSpec<SocketSpec<ReqRes::Worker<
					RequestMessageSpec,
					ResponseMessageSpec,
					SocketFamilyInterface,
					SocketOptions...>>>>>;
		using WorkerSocketSpec::WorkerSocketSpec;
	};
}
