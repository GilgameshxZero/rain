// Provides interfaces implemented by specific commands.
// Commands declare traits by implementing specific
// interfaces.
#pragma once

#include "../../string.hpp"

#include <iostream>

namespace Rain::Networking::Imap {
	class CommandInterface {
		public:
		// All derived classes must provide a (static, constant)
		// command name (e.g. "IDLE", "APPEND") on construction.
		// This implements a form of static polymorphism.
		std::string const name;

		// All commands admit a list of "argument"s/"item"s,
		// whose types may differ. We model this via a shared
		// `CommandData` type.

		CommandInterface(std::string const &name) :
			name{name} {}

		// Base class with virtual functions should admit
		// virtual destructor.
		virtual ~CommandInterface() = default;

		// Command syntax is shared between R/R commands and is
		// implemented here. Since literal arguments may be too
		// big for memory, their buffers must be provided to
		// this interface. Since these buffers do not belong to
		// a socket connection, but rather the command itself
		void sendWith(std::ostream &) {}
		// `recv` parsing is largely shared between R/R, with
		// the exception of the mapping of command names to
		// types. Derived interfaces will complete parsing of
		// the command name, and factory construct the correct
		// derived command type, returning a unique pointer to
		// the derived interface type. The remaining parsing of
		// the command will be completed via dispatch calls
		// within each derived command type. Shared behavior of
		// derived interfaces will be implemented here.
		void recvWith(std::istream &) {}

		// Command dispatch is implemented by derived interfaces
		// `RequestCommandInterface` and
		// `ResponseCommandInterface`, to dispatch to *Worker
		// and *Client types, respectively. It is not declared
		// here.
		//
		// Dispatch does not require the entire command the be
		// read into memory, as command literals may be
		// arbitrarily long. Instead, multiple dispatches will
		// be called (by the derived classes for each command),
		// one for each argument. The dispatcher will provide a
		// separate `istream` with a bounded `streambuf`
		// matching the argument boundary. Any remaining bytes
		// left in the `istream` will be discarded after
		// dispatch, and thus be discarded from the base
		// istream.
		//
		// Since parsing logic is shared between all bounded
		// argument streams, the stream is provided in this base
		// interface. This stream is intended to parse a literal
		// argument. All other arguments are bounded below 1K
		// bytes, and can be served via memory.
		class LiteralStreamBuf {};
	};
}
