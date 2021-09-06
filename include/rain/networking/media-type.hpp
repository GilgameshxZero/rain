// MIME Media Type, used in both SMTP and HTTP.
#pragma once

#include "../string/string.hpp"

#include <unordered_map>

namespace Rain::Networking {
	class MediaType {
		public:
		enum Type { TEXT = 0, IMAGE, AUDIO, APPLICATION, FONT };
		enum Value {
			PLAIN = 0,
			HTML,
			CSS,
			JAVASCRIPT,
			MARKDOWN,

			VND_MICROSOFT_ICON,
			JPEG,
			PNG,
			GIF,
			SVG_XML,

			MPEG,

			OCTET_STREAM,
			PDF,
			ZIP,

			WOFF,
			WOFF2,
			TTF
		};

		private:
		Value value;

		// fromStr includes the type/subtype format but also the file extension
		// format.
		inline static std::unordered_map<
			std::string,
			Value,
			Rain::String::HashCaseAgnostic,
			Rain::String::EqualCaseAgnostic> const _fromStr{
			{"text/plain", PLAIN},
			{".txt", PLAIN},
			{"text/html", HTML},
			{".html", HTML},
			{"text/css", CSS},
			{".css", CSS},
			{"text/javascript", JAVASCRIPT},
			{".js", JAVASCRIPT},
			{"text/markdown", MARKDOWN},
			{".md", MARKDOWN},

			{"image/vnd.microsoft.icon", VND_MICROSOFT_ICON},
			{".ico", VND_MICROSOFT_ICON},
			{"image/jpeg", JPEG},
			{".jpg", JPEG},
			{".jpeg", JPEG},
			{"image/png", PNG},
			{".png", PNG},
			{"image/gif", GIF},
			{".gif", GIF},
			{"image/svg+xml", SVG_XML},
			{".svg", SVG_XML},

			{"audio/mpeg", MPEG},
			{".mp3", MPEG},

			{"application/octet-stream", OCTET_STREAM},
			{"application/pdf", PDF},
			{".pdf", PDF},
			{"application/zip", ZIP},
			{".zip", ZIP},

			{"font/woff", WOFF},
			{".woff", WOFF},
			{"font/woff2", WOFF2},
			{".woff2", WOFF2},
			{"font/ttf", TTF},
			{".ttf", TTF},
		};

		public:
		// Only for MediaType, types not found will default to octet-stream.
		static Value fromStr(std::string const &str) {
			auto it = MediaType::_fromStr.find(str);
			if (it == MediaType::_fromStr.end()) {
				return OCTET_STREAM;
			}
			return it->second;
		}

		// parameter is copy-constructed since it is likely light.
		std::string const parameter;

		MediaType(
			Value value = OCTET_STREAM,
			std::string const &parameter = "") noexcept
				: value(value), parameter(parameter) {}
		MediaType(std::string const &str)
				: value(MediaType::fromStr(str.substr(0, str.find(';')))),
					parameter(str.substr(std::min(str.find(';'), str.length() - 1) + 1)) {
		}
		MediaType(std::string const &value, std::string const &parameter)
				: value(MediaType::fromStr(value)), parameter(parameter) {}
		operator Value() const noexcept { return this->value; }
		explicit operator bool() = delete;
		operator std::string() const noexcept {
			std::string subtypeStr = [this]() {
				switch (this->value) {
					case PLAIN:
						return "text/plain";
					case HTML:
						return "text/html";
					case CSS:
						return "text/css";
					case JAVASCRIPT:
						return "text/javascript";
					case MARKDOWN:
						return "text/markdown";

					case VND_MICROSOFT_ICON:
						return "image/vnd.microsoft.icon";
					case JPEG:
						return "image/jpeg";
					case PNG:
						return "image/png";
					case GIF:
						return "image/gif";
					case SVG_XML:
						return "image/svg+xml";

					case MPEG:
						return "audio/mpeg";

					case OCTET_STREAM:
					default:
						return "application/octet-stream";
					case PDF:
						return "application/pdf";
					case ZIP:
						return "application/zip";

					case WOFF:
						return "font/woff";
					case WOFF2:
						return "font/woff2";
					case TTF:
						return "font/ttf";
				}
			}();
			return this->parameter.empty() ? subtypeStr
																		 : subtypeStr + "; " + this->parameter;
		}

		Type toType() const noexcept {
			switch (this->value) {
				case PLAIN:
				default:
				case HTML:
				case CSS:
				case JAVASCRIPT:
				case MARKDOWN:
					return Type::TEXT;

				case VND_MICROSOFT_ICON:
				case JPEG:
				case PNG:
				case GIF:
				case SVG_XML:
					return Type::IMAGE;

				case MPEG:
					return Type::AUDIO;

				case OCTET_STREAM:
				case PDF:
				case ZIP:
					return Type::APPLICATION;

				case WOFF:
				case WOFF2:
				case TTF:
					return Type::FONT;
			}
		}
	};
}
