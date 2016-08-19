/*
 * cli.cpp v1.0.0
 * <https://github.com/Bloutiouf/cli.cpp>
 * Copyright (c) 2016 Jonathan Giroux "Bloutiouf"
 * MIT License <https://opensource.org/licenses/MIT>
*/

#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <vector>

#ifndef CLI_ERROR_BUFFER_SIZE
#define CLI_ERROR_BUFFER_SIZE 256
#endif

namespace cli
{
	template<class T, template <typename A> class Alloc>
	using list_t = std::list<T, Alloc<T>>;

	template <template <typename T> class Alloc = std::allocator>
	class BasicParser;
	typedef BasicParser<> Parser;

	template <template <typename T> class Alloc>
	class Command
	{
	public:
		typedef std::function<int(cli::BasicParser<Alloc> &)> callback_t;

		Command(list_t<const char *, Alloc> &args)
			: _args(args)
			, _description(nullptr)
			, _callback(nullptr)
		{
		}

		Command(list_t<const char *, Alloc> &args,
			const char *name)
			: _args(args)
			, _description(nullptr)
		{
			_aliases.push_back(name);
		}

		Command &alias(const char *value)
		{
			assert(value);
			_aliases.push_back(value);
			return *this;
		}

		Command &description(const char *value)
		{
			_description = value;
			return *this;
		}

		void execute(callback_t callback)
		{
			_callback = callback;
		}

	private:
		list_t<const char *, Alloc> &_args;
		std::list<const char *, Alloc<const char *>> _aliases;
		const char *_description;
		callback_t _callback;

		friend class BasicParser<Alloc>;
	};

	template <template <typename T> class Alloc>
	class Flag
	{
	public:
		Flag(list_t<const char *, Alloc> &args,
			const char *name)
			: _args(args)
			, _description(nullptr)
			, _set(false)
		{
			_aliases.push_back(name);
		}

		Flag &alias(const char *value)
		{
			assert(value);
			_aliases.push_back(value);
			return *this;
		}

		Flag &description(const char *value)
		{
			_description = value;
			return *this;
		}

		bool getValue()
		{
			if (_set)
				return _value;

			auto it = _args.begin();
			++it;
			for (; it != _args.end(); ++it)
			{
				const char *arg = *it;
				if (arg[0] != '-')
					continue;

				do
				{
					arg = &arg[1];
					if (!arg[0])
						goto end;
				} while (arg[0] == '-');

				for (auto alias : _aliases)
				{
					if (!strcmp(arg, alias))
					{
						_args.erase(it);
						_set = true;
						_value = true;
						return true;
					}
				}
			}

		end:
			_set = true;
			_value = false;
			return false;
		}

	private:
		list_t<const char *, Alloc> &_args;
		std::list<const char *, Alloc<const char *>> _aliases;
		const char *_description;
		bool _value;
		bool _set;

		friend class BasicParser<Alloc>;
	};

	template <template <typename T> class Alloc>
	class Option
	{
	public:
		Option(list_t<const char *, Alloc> &args,
			list_t<char *, Alloc> &errors,
			const char *name)
			: _args(args)
			, _errors(errors)
			, _defaultValue(nullptr)
			, _description(nullptr)
			, _required(false)
			, _set(false)
		{
			_aliases.push_back(name);
		}

		Option &alias(const char *value)
		{
			assert(value);
			_aliases.push_back(value);
			return *this;
		}

		Option &defaultValue(const char *value)
		{
			_defaultValue = value;
			return *this;
		}

		Option &description(const char *value)
		{
			_description = value;
			return *this;
		}

		Option &required()
		{
			_required = true;
			return *this;
		}

		const char *getValue()
		{
			if (_set)
				return _value;

			auto it = _args.begin();
			++it;
			for (; it != _args.end(); ++it)
			{
				const char *arg = *it;
				if (arg[0] != '-')
					continue;

				do
				{
					arg = &arg[1];
					if (!arg[0])
						goto end;
				} while (arg[0] == '-');

				for (auto alias : _aliases)
				{
					if (!strcmp(arg, alias))
					{
						it = _args.erase(it);
						if (it == _args.end())
							goto end;

						_set = true;
						_value = *it;
						_args.erase(it);
						return _value;
					}
				}
			}

		end:
			if (_required)
			{
				const char *alias = _aliases.front();
				size_t n = strlen(alias) + 11;
				auto alloc = Alloc<char>();
				char *error = alloc.allocate(n);
				snprintf(error, n, "-%s required", alias);
				_errors.push_back(error);
			}

			_set = true;
			_value = _defaultValue;
			return _defaultValue;
		}

		template <typename T>
		T getValueAs()
		{
			const char *value = getValue();
			T t;
			if (value)
			{
				std::istringstream iss(value);
				iss >> t;
			}
			return t;
		}

	private:
		list_t<const char *, Alloc> &_args;
		list_t<char *, Alloc> &_errors;
		std::list<const char *, Alloc<const char *>> _aliases;
		const char *_defaultValue;
		const char *_description;
		const char *_value;
		bool _required;
		bool _set;

		friend class BasicParser<Alloc>;
	};

	template <template <typename T> class Alloc>
	class BasicParser
	{
	public:
		typedef std::basic_string<char, std::char_traits<char>, Alloc<char>> string;
		typedef std::basic_ostringstream<char, std::char_traits<char>, Alloc<char>> ostringstream_t;

		typedef Command<Alloc> command_t;
		typedef Flag<Alloc> flag_t;
		typedef Option<Alloc> option_t;

		BasicParser(int argc, const char **argv, std::ostream &helpStream = std::cout, std::ostream &errorStream = std::cerr)
			: _defaultCommand(_args)
			, _hasDefaultCommand(false)
			, _helpStream(helpStream)
			, _errorStream(errorStream)
		{
			for (int i = 0; i < argc; ++i)
			{
				assert(argv[i]);
				_args.push_back(argv[i]);
			}
		}

		~BasicParser()
		{
			if (!_helpMessage.str().empty())
			{
				showHelp();
			}

			if (!_errors.empty())
			{
				showErrors();
			}
		}

		command_t &command(const char *name)
		{
			assert(name);
			_commands.push_back(command_t(_args, name));
			return _commands.back();
		}

		command_t &defaultCommand()
		{
			_hasDefaultCommand = true;
			return _defaultCommand;
		}

		flag_t &flag(const char *name)
		{
			assert(name);
			_flags.push_back(flag_t(_args, name));
			return _flags.back();
		}

		option_t &option(const char *name)
		{
			assert(name);
			_options.push_back(option_t(_args, _errors, name));
			return _options.back();
		}

		ostringstream_t &help(bool showHelp)
		{
			return showHelp ? _helpMessage : _noHelpMessage;
		}

		ostringstream_t &help(flag_t &showHelp)
		{
			return help(showHelp.getValue());
		}

		ostringstream_t &help()
		{
			flag_t flag = defaultHelpFlag();
			return help(flag);
		}

		flag_t defaultHelpFlag()
		{
			return flag("help")
				.alias("h")
				.alias("?")
				.description("Show help");
		}

		void getRemainingArguments(int &argc, const char **argv) const
		{
			argc = (int)_args.size();

			int i = 0;
			bool dashEncountered = false;
			for (auto arg : _args)
			{
				if (!dashEncountered && arg[0] == '-')
				{
					const char *dashArg = arg;
					do
					{
						dashArg = &dashArg[1];
						if (!dashArg[0])
						{
							dashEncountered = true;
							--argc;
							break;
						}
					} while (dashArg[0] == '-');

					if (dashEncountered)
						continue;
				}

				argv[i] = arg;
				++i;
			}
		}

		bool executeCommand(int &result)
		{
			auto it = _args.begin();

			do {
				++it;

				if (it == _args.end())
				{
					if (_hasDefaultCommand)
					{
						if (_defaultCommand._callback)
						{
							result = callCommand(_defaultCommand._callback);
							return true;
						}
						else
						{
							const char *alias = _defaultCommand._aliases.front();
							size_t n = strlen(alias) + 18;
							auto alloc = Alloc<char>();
							char *error = alloc.allocate(n);
							snprintf(error, n, "-%s has no callback", alias);
							_errors.push_back(error);
						}
					}

					size_t n = 17;
					auto alloc = Alloc<char>();
					char *error = alloc.allocate(n);
					snprintf(error, n, "No command given");
					_errors.push_back(error);

					return false;
				}
			} while ((*it)[0] == '-');

			const char *commandName = *it;
			_args.erase(it);

			if (_hasDefaultCommand)
			{
				if (_defaultCommand._callback)
				{
					for (auto alias : _defaultCommand._aliases)
					{
						if (!strcmp(commandName, alias))
						{
							result = callCommand(_defaultCommand._callback);
							return true;
						}
					}
				}
				else
				{
					const char *alias = _defaultCommand._aliases.front();
					size_t n = strlen(alias) + 18;
					auto alloc = Alloc<char>();
					char *error = alloc.allocate(n);
					snprintf(error, n, "-%s has no callback", alias);
					_errors.push_back(error);
				}
			}

			for (auto &command : _commands)
			{
				if (command._callback)
				{
					for (auto alias : command._aliases)
					{
						if (!strcmp(commandName, alias))
						{
							result = callCommand(command._callback);
							return true;
						}
					}
				}
				else
				{
					const char *alias = command._aliases.front();
					size_t n = strlen(alias) + 18;
					auto alloc = Alloc<char>();
					char *error = alloc.allocate(n);
					snprintf(error, n, "-%s has no callback", alias);
					_errors.push_back(error);
				}
			}

			size_t n = strlen(commandName) + 17;
			auto alloc = Alloc<char>();
			char *error = alloc.allocate(n);
			snprintf(error, n, "Unknown command %s", commandName);
			_errors.push_back(error);

			return false;
		}

		void showHelp(std::ostream &stream)
		{
			string helpMessage = _helpMessage.str();
			if (!helpMessage.empty())
			{
				stream << helpMessage << "\n\n";
				helpMessage.clear();
				_helpMessage.str(helpMessage);
			}

			if (_commands.size() || _hasDefaultCommand)
			{
				stream << "Commands:\n";

				if (_hasDefaultCommand)
				{
					stream << "  <default>";
					for (auto &alias : _defaultCommand._aliases)
					{
						stream << ", " << alias;
					}
					if (_defaultCommand._description)
						stream << "  " << _defaultCommand._description;
					stream << "\n";
				}

				for (auto &command : _commands)
				{
					stream << "  ";
					bool first = true;
					for (auto &alias : command._aliases)
					{
						if (first)
							first = false;
						else
							stream << ", ";
						stream << alias;
					}
					if (command._description)
						stream << "  " << command._description;
					stream << "\n";
				}

				stream << "\n";
			}

			if (_flags.size())
			{
				stream << "Flags:\n";
				for (auto &flag : _flags)
				{
					stream << "  -";
					bool first = true;
					for (auto &alias : flag._aliases)
					{
						if (first)
							first = false;
						else
							stream << ", -";
						stream << alias;
					}
					if (flag._description)
						stream << "  " << flag._description;
					stream << "\n";
				}
				stream << "\n";
			}

			if (_options.size())
			{
				stream << "Options:\n";
				for (auto &option : _options)
				{
					stream << "  -";
					bool first = true;
					for (auto &alias : option._aliases)
					{
						if (first)
							first = false;
						else
							stream << ", -";
						stream << alias;
					}
					if (option._required)
						stream << " (required)";
					if (option._description)
						stream << "  " << option._description;
					if (option._defaultValue)
						stream << " [default: " << option._defaultValue << "]";
					stream << "\n";
				}
				stream << "\n";
			}

		}

		void showHelp()
		{
			showHelp(_helpStream);
		}

		bool hasErrors() const
		{
			return !_errors.empty();
		}

		void reportError(const char *format, ...)
		{
			auto alloc = Alloc<char>();
			auto buffer = alloc.allocate(CLI_ERROR_BUFFER_SIZE);

			va_list args;
			va_start(args, format);
			vsnprintf(buffer, CLI_ERROR_BUFFER_SIZE, format, args);
			va_end(args);

			_errors.push_back(buffer);
		}

		void showErrors(std::ostream &stream)
		{
			auto alloc = Alloc<char>();
			for (auto error : _errors)
			{
				stream << error << std::endl;
				alloc.deallocate(error, strlen(error));
			}
			_errors.clear();
		}

		void showErrors()
		{
			showErrors(_errorStream);
		}

	private:
		typedef std::function<int(cli::BasicParser<Alloc> &)> callback_t;

		list_t<const char *, Alloc> _args;
		list_t<char *, Alloc> _errors;
		list_t<command_t, Alloc> _commands;
		list_t<flag_t, Alloc> _flags;
		list_t<option_t, Alloc> _options;
		command_t _defaultCommand;
		bool _hasDefaultCommand;

		std::ostream &_helpStream;
		std::ostream &_errorStream;

		ostringstream_t _helpMessage;
		ostringstream_t _noHelpMessage;

		BasicParser(BasicParser<Alloc> &other)
			: _defaultCommand(_args)
			, _hasDefaultCommand(false)
			, _helpStream(other._helpStream)
			, _errorStream(other._errorStream)
		{
			bool dashEncountered = false;
			for (auto arg : other._args)
			{
				if (!dashEncountered && arg[0] == '-')
				{
					const char *dashArg = arg;
					do
					{
						dashArg = &dashArg[1];
						if (!dashArg[0])
						{
							dashEncountered = true;
							break;
						}
					} while (dashArg[0] == '-');

					if (dashEncountered)
						continue;
				}

				_args.push_back(arg);
			}

			_errors = other._errors;
			other._errors.clear();
		}

		int callCommand(callback_t callback)
		{
			Parser subParser(*this);
			return callback(subParser);
		}
	};

}
