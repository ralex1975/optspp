#pragma once

#include "../scheme.hpp"

namespace optspp {
  namespace scheme {
    entity::entity(const entity& other) :
      kind_(other.kind_),
      siblings_group_(other.siblings_group_),
      description_(other.description_),
      long_names_(other.long_names_),
      short_names_(other.short_names_),
      max_count_(other.max_count_),
      min_count_(other.min_count_),
      is_positional_(other.is_positional_),
      known_values_(other.known_values_),
      default_values_(other.default_values_),
      any_value_(other.any_value_) {
      for (const auto& p : other.pending_) {
        auto c = std::make_shared<entity>(*p);
        pending_.push_back(c);
        pending_siblings_group_[c] = other.pending_siblings_group_.at(p);
      }
    }
    
    entity::entity(const KIND kind) :
      kind_(kind) {
    }

    void entity::set_siblings_group(const SIBLINGS_GROUP group) {
      siblings_group_ = group;
    }

    void entity::set_positional() {
      is_positional_ = true;
    }


    void entity::set_named() {
      is_positional_ = false;
    }

    void entity::set_value(const std::vector<std::string>& vs) {
      known_values_ = vs;
      any_value_ = false;
    }

    void entity::set_value(any) {
      known_values_ = optional<std::vector<std::string>>();
      any_value_ = true;
    }

    template <>
    void entity::apply_property(name p) {
      if (kind_ == KIND::ARGUMENT) {
        if (p.long_names) {
          if (!long_names_) long_names_ = std::vector<std::string>{};
          auto lhs = *long_names_;
          auto rhs = *p.long_names;
          lhs.push_back(rhs[0]);
          for (auto it = rhs.begin() + 1; it != rhs.end(); ++it) 
            if (std::find(lhs.begin(), lhs.end(), *it) == lhs.end()) lhs.push_back(*it);
        }
        if (p.short_names) {
          if (!short_names_) short_names_ = std::vector<char>{};
          auto lhs = *short_names_;
          auto rhs = *p.short_names;
          lhs.push_back(rhs[0]);
          for (auto it = rhs.begin() + 1; it != rhs.end(); ++it) 
            if (std::find(lhs.begin(), lhs.end(), *it) == lhs.end()) lhs.push_back(*it);
        }        
      }
    }

    template <>
    void entity::apply_property(default_values p) {
      if (kind_ == KIND::ARGUMENT) {
        if (p.values) {
          if (!default_values_) default_values_ = std::vector<std::string>{};
          auto lhs = *default_values_;
          for (const auto& s : *p.values) {
            lhs.push_back(s);
          }
        }
      }
    }

    template <>
    void entity::apply_property(implicit_values p) {
      if (kind_ == KIND::ARGUMENT) {
        if (p.values) {
          if (!implicit_values_) implicit_values_ = std::vector<std::string>{};
          auto lhs = *implicit_values_;
          for (const auto& s : *p.values) {
            lhs.push_back(s);
          }
        }
      }
    }
    
    template <>
    void entity::apply_property(description p) {
      if (p.text) {
        description_ = p.text;
      }
    }

    template <>
    void entity::apply_property(min_count p) {
      if (p.count) {
        min_count_ = p.count;
      }
    }

    template <>
    void entity::apply_property(max_count p) {
      if (p.count) {
        min_count_ = p.count;
      }
    }
    
  }
}
