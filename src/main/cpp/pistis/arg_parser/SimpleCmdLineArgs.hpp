#ifndef __PISTIS__ARG_PARSER__SIMPLECMDLINEARGS_HPP__
#define __PISTIS__ARG_PARSER__SIMPLECMDLINEARGS_HPP__

#include <pistis/exceptions/ItemExistsError.hpp>
#include <pistis/exceptions/NoSuchItem.hpp>
#include <pistis/util/NumUtil.hpp>
#include <pistis/util/StringUtil.hpp>
#include <pistis/arg_parser/AbstractCmdLineArgs.hpp>
#include <pistis/arg_parser/CmdLineArgGenerator.hpp>
#include <exception>
#include <functional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace pistis {
  namespace arg_parser {

      class SimpleCmdLineArgs : public AbstractCmdLineArgs {
      protected:
	class FormatError : public pistis::exceptions::PistisException {
	public:
	  FormatError(const std::string& details);
	  FormatError(const std::string& value, const std::string& details);

	  const std::string& value() const { return value_; }
	  const std::string& details() const { return details_; }

	private:
	  std::string value_;
	  std::string details_;

	  static std::string createMessage_(const std::string& value,
					    const std::string& details);
	};

	template <typename Value>
	class ValueMap {
	public:
	  ValueMap(): values_() { }
	  ValueMap(const ValueMap<Value>& other): values_(other.values_) { }
	  ValueMap(ValueMap&& other): values_(std::move(other.values_)) { }
	  ValueMap(const std::initializer_list<
		       std::pair<std::string, Value>
		   >& values):
	      values_(values) {
	  }
	  ~ValueMap() { }

	  std::vector<std::string> allKeys() const {
	    std::vector<std::string> keys;
	    keys.reserve(values_.size());
	    for (auto i= values_.begin(); i != values_.end(); ++i) {
	      keys.push_back(i->first);
	    }
	    return keys;
	  }
	  bool hasValue(const std::string& key) const {
	    return values_.find(key) != values_.end();
	  }
	  Value operator[](const std::string& key) const {
	    auto i= values_.find(key);
	    if (i == values_.end()) {
	      std::vector<std::string> keys(allKeys());
	      std::ostringstream msg;
	      msg << "Legal values are \""
		  << util::join(keys.begin(), keys.end(), "\", \"")
		  << "\"";
	      throw FormatError(key, msg.str());
	    }
	    return i->second;
	  }
	  void setValue(const std::string& key, const Value& value) {
	    if (hasValue(key)) {
	      throw exceptions::ItemExistsError(key, PISTIS_EX_HERE);
	    }
	    values_.insert(std::make_pair(key, value));
	  }

	  ValueMap<Value>& operator=(const ValueMap<Value>& other) {
	    values_= other.values_;
	    return *this;
	  }

	  ValueMap<Value>& operator=(ValueMap<Value>&& other) {
	    values_= std::move(other.values_);
	    return *this;
	  }

	private:
	  std::unordered_map<std::string, Value> values_;
	};

	class ArgHandler {
	public:
	  ArgHandler(const std::string& argName, const std::string& description,
		     bool isRequired, bool isFinal);

	  const std::string& argName() const { return argName_; }
	  const std::string& description() const { return description_; }
	  bool required() const { return required_; }
	  bool final() const { return final_; }
	  bool found() const { return found_; }

	  std::string fullName() const;

	  void setFound(bool v) { found_= v; }
	  virtual void handleValue(CmdLineArgGenerator& args,
				   const std::string& arg) = 0;

	private:
	  std::string argName_;
	  std::string description_;
	  bool required_;
	  bool final_;
	  bool found_;
	};

	template <typename Delegate>
	class DelegatingArgHandler : public ArgHandler {
	public:
	  DelegatingArgHandler(const std::string& argName,
			       const std::string& description,
			       bool isRequired,
			       bool isFinal,
			       const Delegate& delegate):
	    ArgHandler(argName, description, isRequired, isFinal),
	    delegate_(delegate) {
	  }

	  virtual void handleValue(CmdLineArgGenerator& args,
				   const std::string& arg) {
	    delegate_(args, arg);
	  }

	private:
	  Delegate delegate_;
	};

	template <typename Value>
	class ArgFormatter {
	  static_assert(sizeof(Value) == 0,
			"Unsupported destination value type");
	};


	typedef std::unordered_map<std::string, ArgHandler*> HandlerMapType;
	typedef std::vector<ArgHandler*> HandlerListType;

      public:
	SimpleCmdLineArgs();
	virtual ~SimpleCmdLineArgs();

      protected:
	template <typename Formatter>
	static auto formatUsingFn(const std::string& value,
				  const Formatter& f) {
	  try {
	    return f(value);
	  } catch(const FormatError& e) {
	    throw;
	  } catch(const std::exception& e) {
	    throw FormatError(value, e.what());
	  } catch(...) {
	    throw FormatError(value, std::string());
	  }
	}

	template <typename Function>
	static void splitAndApply(const std::string& value,
				  const std::string& separator,
				  bool allowEmpty,
				  const Function& f) {
	  static const util::SplitIterator END_OF_SPLIT;
	  if (!value.empty()) {
	    for (auto i= util::SplitIterator(value, separator);
		 i != END_OF_SPLIT;
		 ++i) {
	      try {
		f(*i);
	      } catch(const FormatError& e) {
		throw;
	      } catch(const std::exception& e) {
		throw FormatError(*i, e.what());
	      } catch(...) {
		throw FormatError(*i, std::string());
	      }
	    }
	  } else if (!allowEmpty) {
	    throw FormatError("Value is empty");
	  }
	}
				  
	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       Value& v) {
	  ArgHandler* h =
	      createDelegate_(argName, description, required, true,
			    [&v](CmdLineArgGenerator& args,
				 const std::string& argName) -> void {
	        v = ArgFormatter<Value>::format(args.next(argName));
	      });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       std::vector<Value>& v) {
	  ArgHandler *h =
	      createDelegate_(argName, description, required, true,
			      [&v](CmdLineArgGenerator& args,
				   const std::string& argName) -> void {
	        v.push_back(ArgFormatter<Value>::format(args.next(argName)));
	      });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::string& separator,
			       bool allowEmpty,
			       std::vector<Value>& v) {
	  ArgHandler *h =
	    createDelegate_(
	        argName, description, required, true,
		[&v, separator, allowEmpty, this](
		    CmdLineArgGenerator& args, const std::string& argName
		) -> void {
	          splitAndApply(args.next(argName), separator, allowEmpty,
				[&v](const std::string& value) -> void {
		    v.push_back(ArgFormatter<Value>::format(value));
	          });
		}
	    );
	  registerHandler_(h);
	}
	
	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       std::unordered_set<Value>& v) {
	  ArgHandler *h=
	      createDelegate_(argName, description, required, true,
                              [&v](CmdLineArgGenerator& args,
				   const std::string& argName) -> void {
	        v.insert(ArgFormatter<Value>::format(args.next(argName)));
	      });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::string& separator,
			       bool allowEmpty,
			       std::unordered_set<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
                            [&v, separator, allowEmpty, this](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      splitAndApply(args.next(argName), separator, allowEmpty,
			    [&v](const std::string& value) {
		v.insert(ArgFormatter<Value>::format(value));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInRange_(const std::string& argName,
				      const std::string& description,
				      bool required, Value minValue,
				      Value maxValue, Value& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [&v, minValue, maxValue](
			         CmdLineArgGenerator& args,
				 const std::string& argName
			    ) -> void {
	      v= ArgFormatter<Value>::format(args.next(argName), minValue,
					     maxValue);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInRange_(const std::string& argName,
				      const std::string& description,
				      bool required,
				      Value minValue, Value maxValue,
				      std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [&v, minValue, maxValue](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      v.push_back(ArgFormatter<Value>::format(args.next(argName),
						      minValue, maxValue));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInRange_(const std::string& argName,
				      const std::string& description,
				      bool required,
				      const std::string& separator,
				      bool allowEmpty,
				      Value minValue, Value maxValue,
				      std::vector<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(
	        argName, description, required, true,
		[&v, separator, allowEmpty, minValue, maxValue, this](
		    CmdLineArgGenerator& args,
		    const std::string& argName
	        ) -> void {
	          splitAndApply(args.next(argName), separator, allowEmpty,
				[&v, minValue, maxValue](
				    const std::string& value
				) -> void {
	            v.push_back(ArgFormatter<Value>::format(value, minValue,
							    maxValue));
	          });
		}
	    );
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInRange_(const std::string& argName,
				      const std::string& description,
				      bool required, Value minValue,
				      Value maxValue,
				      std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [&v, minValue, maxValue](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      v.insert(ArgFormatter<Value>::format(args.next(argName),
						   minValue, maxValue));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInRange_(const std::string& argName,
				      const std::string& description,
				      bool required,
				      const std::string& separator,
				      bool allowEmpty,
				      Value minValue, Value maxValue,
				      std::unordered_set<Value>& v) {
	  ArgHandler *h =
	    createDelegate_(
		argName, description, required, true,
		[&v,separator, allowEmpty, minValue, maxValue, this](
		    CmdLineArgGenerator& args,
		    const std::string& argName
	        ) -> void {
	          splitAndApply(args.next(argName), separator, allowEmpty,
				[&v, minValue, maxValue](
				    const std::string& value
				) -> void {
	            v.insert(ArgFormatter<Value>::format(value, minValue,
							 maxValue));
	          }
	        );
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInSet_(const std::string& argName,
				    const std::string& description,
				    bool required,
				    const std::unordered_set<Value>&
				        legalValues,
				    Value& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [&v, legalValues](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      v= ArgFormatter<Value>::format(args.next(argName), legalValues);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInSet_(const std::string& argName,
				    const std::string& description,
				    bool required,
				    const std::unordered_set<Value>&
				        legalValues,
				    std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [&v, legalValues](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      v.push_back(ArgFormatter<Value>::format(args.next(argName),
						      legalValues));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInSet_(const std::string& argName,
				    const std::string& description,
				    bool required,
				    const std::string& separator,
				    bool allowEmpty,
				    const std::unordered_set<Value>&
				        legalValues,
				    std::vector<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
                            [&v, separator, allowEmpty, legalValues, this](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      splitAndApply(args.next(argName), separator, allowEmpty,
			    [&v, legalValues](const std::string& value) {
	        v.push_back(ArgFormatter<Value>::format(value, legalValues));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInSet_(const std::string& argName,
				    const std::string& description,
				    bool required,
				    const std::unordered_set<Value>&
				        legalValues,
				    std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [&v, legalValues](CmdLineArgGenerator& args,
					      const std::string& argName) {
	      v.insert(ArgFormatter<Value>::format(args.next(argName),
						   legalValues));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArgInSet_(const std::string& argName,
				    const std::string& description,
				    bool required,
				    const std::string& separator,
				    bool allowEmpty,
				    const std::unordered_set<Value>&
				        legalValues,
				    std::unordered_set<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
                            [&v, separator, allowEmpty, legalValues, this](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) -> void {
	      splitAndApply(args.next(argName), separator, allowEmpty,
			    [&v, legalValues](const std::string& value) {
	        v.insert(ArgFormatter<Value>::format(value, legalValues));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const ValueMap<Value>& valueMap,
			       Value& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
			    [&v, valueMap](CmdLineArgGenerator& args,
					   const std::string& argName) {
	      v= valueMap[args.next(argName)];
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const ValueMap<Value>& valueMap,
			       std::vector<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
			    [&v, valueMap](CmdLineArgGenerator& args,
					   const std::string& argName) {
	      v.push_back(valueMap[args.next(argName)]);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::string& separator,
			       bool allowEmpty,
			       const ValueMap<Value>& valueMap,
			       std::vector<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
			    [&v, valueMap, separator, allowEmpty, this](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) {
	      splitAndApply(args.next(argName), separator, allowEmpty,
			    [&v, valueMap](const std::string& value) {
	        v.push_back(valueMap[value]);
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const ValueMap<Value>& valueMap,
			       std::unordered_set<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
			    [&v, valueMap](CmdLineArgGenerator& args,
					   const std::string& argName) {
	      v.insert(valueMap[args.next(argName)]);
	    });
          registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::string& separator,
			       bool allowEmpty,
			       const ValueMap<Value>& valueMap,
			       std::unordered_set<Value>& v) {
	  ArgHandler *h=
	    createDelegate_(argName, description, required, true,
			    [&v, valueMap, separator, allowEmpty, this](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) {
	      splitAndApply(args.next(argName), separator, allowEmpty,
		 	    [&v,valueMap](const std::string& value) {
	        v.insert(valueMap[value]);
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::function<Value (const std::string&)>&
			           format,
			       Value& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [format, this, &v](CmdLineArgGenerator& args,
					       const std::string& argName) {
	      v= SimpleCmdLineArgs::formatUsingFn(args.next(argName), format);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::function<
			           Value (const std::string&)
			       >& format,
			       std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [format, this, &v](CmdLineArgGenerator& args,
					       const std::string& argName) {
	      v.push_back(formatUsingFn(args.next(argName), format));
	    });
          registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::string& separator,
			       bool allowEmpty,
			       const std::function<
			           Value (const std::string&)
			       >& format,
			       std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [format, separator, allowEmpty, this, &v](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) {
	      splitAndApply(args.next(argName), separator, allowEmpty,
			    [format, this, &v](const std::string& value) {
	        v.push_back(formatUsingFn(value, format));
	      });
	    });
          registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::function<
			           Value (const std::string&)
			       >& format,
			       std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [format, this, &v](CmdLineArgGenerator& args,
					       const std::string& argName) {
	      v.insert(formatUsingFn(args.next(argName), format));
	    });
          registerHandler_(h);
	}

	template <typename Value>
	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::string& separator,
			       bool allowEmpty,
			       const std::function<
			           Value (const std::string&)
			       >& format,
			       std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(argName, description, required, true,
			    [format, separator, allowEmpty, this, &v](
			        CmdLineArgGenerator& args,
				const std::string& argName
			    ) {
	      splitAndApply(args.next(argName), separator, allowEmpty,
			    [format, this, &v](const std::string& value) {
	        v.insert(formatUsingFn(value, format));
	      });
	    });
          registerHandler_(h);
	}

	void registerNamedArg_(const std::string& argName,
			       const std::string& description,
			       bool required,
			       const std::function<
			           void (CmdLineArgGenerator&,
					 const std::string&)
			       >& handler);

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 Value& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v](CmdLineArgGenerator& args,
				 const std::string& argValue) -> void {
	      v = ArgFormatter<Value>::format(argValue);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v](CmdLineArgGenerator& args,
				 const std::string& argValue) -> void {
	      v.push_back(ArgFormatter<Value>::format(argValue));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::string& separator,
				 std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v,separator,this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v](const std::string& value) -> void {
	        v.push_back(ArgFormatter<Value>::format(value));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v](CmdLineArgGenerator& args,
				 const std::string& argValue) -> void {
	      v.insert(ArgFormatter<Value>::format(argValue));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::string& separator,
				 std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, separator, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v](const std::string& value) -> void {
	        v.insert(ArgFormatter<Value>::format(value));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInRange_(const std::string& description,
					bool required,
					Value minValue, Value maxValue,
					Value& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, minValue, maxValue](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      v= ArgFormatter<Value>::format(argValue, minValue, maxValue);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInRange_(const std::string& description,
					bool required,
					Value minValue, Value maxValue,
					std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, minValue, maxValue](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      v.push_back(ArgFormatter<Value>::format(argValue, minValue,
						      maxValue));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInRange_(const std::string& description,
					bool required,
					const std::string& separator,
					Value minValue, Value maxValue,
					std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, separator, minValue, maxValue, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v, minValue, maxValue](
			        const std::string& value
			    ) -> void {
	        v.push_back(ArgFormatter<Value>::format(value, minValue,
							maxValue));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInRange_(const std::string& description,
					bool required,
					Value minValue, Value maxValue,
					std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, minValue, maxValue](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      v.insert(ArgFormatter<Value>::format(argValue, minValue,
						   maxValue));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInRange_(const std::string& description,
					bool required,
					const std::string& separator,
					Value minValue, Value maxValue,
					std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, separator, minValue, maxValue, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v, minValue, maxValue](
			        const std::string& value
			    ) -> void {
	        v.insert(ArgFormatter<Value>::format(value, minValue,
						     maxValue));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInSet_(const std::string& description,
				      bool required,
				      const std::unordered_set<Value>&
				          legalValues,
				      Value& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, legalValues](CmdLineArgGenerator& args,
					      const std::string& argValue) {
	      v = ArgFormatter<Value>::format(argValue, legalValues);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInSet_(const std::string& description,
				      bool required,
				      const std::unordered_set<Value>&
				          legalValues,
				      std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, legalValues](CmdLineArgGenerator& args,
					      const std::string& argValue) {
	      v.push_back(ArgFormatter<Value>::format(argValue, legalValues));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInSet_(const std::string& description,
				      bool required,
				      const std::string& separator,
				      const std::unordered_set<Value>&
				          legalValues,
				      std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, separator, legalValues, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v, legalValues](const std::string& value) {
	        v.push_back(ArgFormatter<Value>::format(value, legalValues));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInSet_(const std::string& description,
				      bool required,
				      const std::unordered_set<Value>&
				          legalValues,
				      std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, legalValues](CmdLineArgGenerator& args,
					      const std::string& argValue) {
	      v.insert(ArgFormatter<Value>::format(argValue, legalValues));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArgInRange_(const std::string& description,
					bool required,
					const std::string& separator,
					const std::unordered_set<Value>&
					    legalValues,
					std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, separator, legalValues, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v, legalValues](const std::string& value) {
	        v.insert(ArgFormatter<Value>::format(value, legalValues));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const ValueMap<Value>& valueMap,
				 Value& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, valueMap](CmdLineArgGenerator& args,
					   const std::string& argValue) {
	      v = valueMap[argValue];
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const ValueMap<Value>& valueMap,
				 std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, valueMap](CmdLineArgGenerator& args,
					   const std::string& argValue) {
	      v.push_back(valueMap[argValue]);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::string& separator,
				 const ValueMap<Value>& valueMap,
				 std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, valueMap, separator, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) -> void {
	      splitAndApply(argValue, separator, false,
			    [&v,valueMap](const std::string& value) {
	        v.push_back(valueMap[value]);
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const ValueMap<Value>& valueMap,
				 std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, valueMap](CmdLineArgGenerator& args,
					   const std::string& argValue) {
	      v.insert(valueMap[argValue]);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::string& separator,
				 const ValueMap<Value>& valueMap,
				 std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, valueMap, separator, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) {
	      splitAndApply(argValue, separator, false,
			    [&v,valueMap](const std::string& value) {
	        v.insert(valueMap[value]);
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::function<
				     Value (const std::string&)
				 >& format,
				 Value& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, this, format](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) {
	      v = formatUsingFn(argValue, format);
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::function<
				     Value (const std::string&)
				 >& format,
				 std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, this, format](CmdLineArgGenerator& args,
					       const std::string& argValue) {
	      v.push_back(formatUsingFn(argValue, format));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::string& separator,
				 const std::function<
				     Value (const std::string&)
				 >& format,
				 std::vector<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, format, this, separator](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) {
	      splitAndApply(argValue, separator, false,
			    [&v, this, format](const std::string& value) {
	        v.push_back(formatUsingFn(value, format));
	      });
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::function<
				     Value (const std::string&)
				 >& format,
				 std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, true,
			    [&v, format, this](CmdLineArgGenerator& args,
					       const std::string& argValue) {
	      v.insert(formatUsingFn(argValue, format));
	    });
	  registerHandler_(h);
	}

	template <typename Value>
	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::string& separator,
				 const std::function<
				     Value (const std::string&)
				 >& format,
				 std::unordered_set<Value>& v) {
	  ArgHandler* h=
	    createDelegate_(std::string(), description, required, false,
			    [&v, format, separator, this](
			        CmdLineArgGenerator& args,
				const std::string& argValue
			    ) {
	      splitAndApply(argValue, separator, false,
			    [&v,format,this](const std::string& value) {
	        v.insert(formatUsingFn(value, format));
	      });
	    });
	  registerHandler_(h);
	}

	void registerUnnamedArg_(const std::string& description,
				 bool required,
				 const std::function<
				     void (CmdLineArgGenerator&,
					   const std::string&)
				 >& handler);
				 
	template <typename Delegate>
        DelegatingArgHandler<Delegate>*
	    createDelegate_(const std::string& argName,
			    const std::string& description,
			    bool required, bool final,
			    const Delegate& delegate) {
	  return new DelegatingArgHandler<Delegate>(argName, description,
						    required, final, delegate);
	}

	void registerHandler_(ArgHandler* handler);

	virtual void init_(int argc, char** argv);
	virtual bool handleNamedArg_(CmdLineArgGenerator& args,
				     const std::string& arg);
	virtual bool handleUnnamedArg_(CmdLineArgGenerator& args,
				       const std::string& arg);
	virtual void check_(const std::string& appName);

	virtual void initValues_();
	virtual void checkValues_();

      private:
	HandlerMapType namedArgs_;
	HandlerListType unnamedArgs_;
	HandlerListType::iterator currentUnnamedArg_;
      };

      template <>
      class SimpleCmdLineArgs::ArgFormatter<int> {
      public:
	static int format(const std::string& value) {
	  std::pair<int64_t, util::NumConversionResult> v =
	    util::toInt64Quietly(value);
	  if (v.second != util::NumConversionResult::OK) {
	    throw FormatError(value, descriptionFor(v.second));
	  }
	  return v.first;
	}

	static int format(const std::string& value, int minValue,
			  int maxValue) {
	  int v= format(value);
	  if ((v < minValue) || (v > maxValue)) {
	    std::ostringstream msg;
	    if (minValue == INT_MIN) {
	      msg << "Value must be less than or equal to " << maxValue;
	    } else if (maxValue == INT_MAX) {
	      msg << "Value must be greater than or equal to " << maxValue;
	    } else {
	      msg << "Value must be between " << minValue << "and "
		  << maxValue << " (inclusive)";
	    }
	    throw FormatError(value, msg.str());
	  }
	  return v;
	}

	static int format(const std::string& value,
			  const std::unordered_set<int>& legalValues) {
	  int v= format(value);
	  if (legalValues.find(v) == legalValues.end()) {
	    std::ostringstream msg;
	    msg << "Legal values are "
		<< util::join(legalValues.begin(), legalValues.end(), ", ");
	    throw FormatError(value, msg.str());
	  }
	  return v;
	}
      };

      template <>
      class SimpleCmdLineArgs::ArgFormatter<double> {
      public:
	static double format(const std::string& value) {
	  std::pair<double, util::NumConversionResult> v=
	    util::toDoubleQuietly(value);
	  if (v.second != util::NumConversionResult::OK) {
	    throw FormatError(value, descriptionFor(v.second));
	  }
	  return v.first;
	}

	static double format(const std::string& value, double minValue,
			     double maxValue) {
	  double v= format(value);
	  if ((v < minValue) || (v > maxValue)) {
	    std::ostringstream msg;
	    if (minValue == -DBL_MAX) {
	      msg << "Value must be less than or equal to " << maxValue;
	    } else if (maxValue == DBL_MAX) {
	      msg << "Value must be greater than or equal to " << maxValue;
	    } else {
	      msg << "Value must be between " << minValue << "and "
		  << maxValue << " (inclusive)";
	    }
	    throw FormatError(value, msg.str());
	  }
	  return v;
	}

	static double format(const std::string& value,
			     const std::unordered_set<double>& legalValues) {
	  double v= format(value);
	  if (legalValues.find(v) == legalValues.end()) {
	    std::ostringstream msg;
	    msg << "Legal values are "
		<< util::join(legalValues.begin(), legalValues.end(), ", ");
	    throw FormatError(value, msg.str());
	  }
	  return v;
	}
      };

      template <>
      class SimpleCmdLineArgs::ArgFormatter<std::string> {
      public:
	static const std::string& format(const std::string& value) {
	  return value;
	}

	static const std::string& format(const std::string& value,
					 const std::string& minValue,
					 const std::string& maxValue) {
	  if ((value < minValue) || (value > maxValue)) {
	    std::ostringstream msg;
	    msg << "Value must be between \"" << minValue << "\" and \""
		<< maxValue << "\" (inclusive)";
	    throw FormatError(value, msg.str());
	  }
	  return value;
	}

	static const std::string& format(
	    const std::string& value,
	    const std::unordered_set<std::string>& legalValues
	) {
	  if (legalValues.find(value) == legalValues.end()) {
	    std::ostringstream msg;
	    msg << "Legal values are \""
		<< util::join(legalValues.begin(), legalValues.end(),
			      "\", \"")
		<< "\"";
	    throw FormatError(value, msg.str());
	  }
	  return value;
	}
      };
	
  }
}
#endif

