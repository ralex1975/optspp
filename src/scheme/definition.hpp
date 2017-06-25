#pragma once

#include "../scheme.hpp"

namespace optspp {
  namespace scheme {
    definition::definition() {
      root_ = std::make_shared<entity>(entity::KIND::NONE);
    }
    
    void definition::parse(const std::vector<std::string>& cmdl_args) {
      if (parsed_) return;
      validate();
      parser p(*this, cmdl_args);
      p.parse();
    }

    void definition::parse(const int argc, char* argv[]) {
      std::vector<std::string> args;
      if (argc > 1) {
        for (int i = 1; i < argc; ++i) args.push_back(std::string(argv[i]));
      }
      parse(args);
    }

    void definition::vertical_name_check(const std::vector<std::string>& taken_long_names,
                                         const std::vector<char>& taken_short_names,
                                         const entity_ptr& e) {
      auto taken_long = taken_long_names;
      auto taken_short = taken_short_names;
      if (e->kind_ == entity::KIND::ARGUMENT) {
        if (e->long_names_) {
          for (const auto& n : *e->long_names_) {
            if (std::find(taken_long.begin(), taken_long.end(), n) != taken_long.end())
              throw std::runtime_error("Scheme validation error: argument's long name should not be used by it's descendant");
          }
          std::copy((*e->long_names_).begin(), (*e->long_names_).end(), std::back_inserter(taken_long));
        }
        if (e->short_names_) {
          for (const auto& n : *e->short_names_) {
            if (std::find(taken_short.begin(), taken_short.end(), n) != taken_short.end())
              throw std::runtime_error("Scheme validation error: argument's short name should not be used by it's descendant");
          }
          std::copy((*e->short_names_).begin(), (*e->short_names_).end(), std::back_inserter(taken_short));
        }
      }
      for (const auto& c : e->pending_) vertical_name_check(taken_long, taken_short, c);
    }

    void definition::validate_entity(const entity_ptr& e) {
      if (e->kind_ == entity::KIND::ARGUMENT) {
        if (e->is_positional_ && *e->is_positional_) {
          if (e->short_names_)
            throw std::runtime_error("Scheme validation error: positional argument has short names");
          if (e->implicit_values_)
            throw std::runtime_error("Scheme validation error: positional argument has implicit values");
          if (e->any_value_ && *e->any_value_) {
            for (const auto& c : e->pending_) {
              if ((c->kind_ == entity::KIND::ARGUMENT) && e->is_positional_ && !*e->is_positional_)
                throw std::runtime_error("Scheme validation error: positional with any value has named child");
            }
          }
        } else {
          bool short_undefined = !e->short_names_ || (e->short_names_ && (*e->short_names_).size() == 0);
          bool long_undefined = !e->long_names_ || (e->long_names_ && (*e->long_names_).size() == 0);
          if (short_undefined && long_undefined )
            throw std::runtime_error("Named argument's both long and short names are empty");
        }
        for (const auto& c : e->pending_) {
          if (c->kind_ != entity::KIND::VALUE) {
            throw std::runtime_error("Scheme validation error: argument entity has non-value child");
          }
        }
        // Implicitly allow any value
        if (std::find_if(e->pending_.begin(), e->pending_.end(), [] (const entity_ptr& v) {
              return v->kind_ == entity::KIND::VALUE;
            }) == e->pending_.end()) {
          std::cout << "Adding ANY value to " << e->all_names_to_string() << "\n";
          e->pending_.push_back(::optspp::value(any()));
        }
      }
      if (e->kind_ == entity::KIND::VALUE) {
        for (const auto& c : e->pending_) {
          if (c->kind_ != entity::KIND::ARGUMENT)
            throw std::runtime_error("Scheme validation error: value entity hash non-argument child");
        }
      }
      for (const auto& c : e->pending_) {
        validate_entity(c);
      }
    }
    
    void definition::validate() const {
      for (const auto& c : root_->pending_) {
        validate_entity(c);
        vertical_name_check(std::vector<std::string>(), std::vector<char>(), c);
      }
    }

    bool definition::is_long_prefix(const std::string& s) const {
      return std::find(long_prefixes_.begin(), long_prefixes_.end(), s) != long_prefixes_.end();
    }

    bool definition::is_short_prefix(const std::string& s) const {
      return std::find(short_prefixes_.begin(), short_prefixes_.end(), s) != short_prefixes_.end();
    }

    const entity_ptr& definition::root() const {
      return root_;
    }

    const std::vector<std::string>& definition::operator[](const std::string& name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) return p.second;
      }
      static std::vector<std::string> empty;
      return empty;
    }

    const std::vector<std::string>& definition::operator[](const char name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) return p.second;
      }
      static std::vector<std::string> empty;
      return empty;
    }

    const std::string& definition::operator()(const std::string& name, const size_t idx) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() < idx) {
            return p.second[idx];
          } else {
            throw std::runtime_error("Index " + std::to_string(idx) + " for argument '" + name + "' is out bounds");
          }
        }
      }
      throw std::runtime_error("Argument '" + name + "' not found");
    }

    const std::string& definition::operator()(const std::string& name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() > 0) {
            return p.second[p.second.size() - 1];
          } else {
            throw std::runtime_error("Argument '" + name + "' has no values");
          }
        }
      }
      throw std::runtime_error("Argument '" + name + "' not found");
    }

    const std::string& definition::operator()(const char name, const size_t idx) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() < idx) {
            return p.second[idx];
          } else {
            throw std::runtime_error("Index " + std::to_string(idx) + " for argument '" + name + "' is out bounds");
          }
        }
      }
      throw std::runtime_error(std::string("Argument '") + name + "' not found");
    }

    const std::string& definition::operator()(const char name) const {
      for (const auto& p : values_) {
        if (p.first->name_matches(name)) {
          if (p.second.size() > 0) {
            return p.second[p.second.size() - 1];
          } else {
            throw std::runtime_error(std::string("Argument '") + name + "' has no values");
          }
        }
      }
      throw std::runtime_error(std::string("Argument '") + name + "' not found");
    }

    
  }
}
