#include <catch.hpp>
#include <optspp/optspp>

#include <map>

SCENARIO("Test optional") {
  using namespace optspp;
  WHEN("Created an empty optional<int>") {
    optional<int> x;
    REQUIRE(x == false);
    THEN("Assign a value") {
      x = 1;
      REQUIRE(x == true);
      REQUIRE(*x == 1);
    } 
  }
  WHEN("Created initialized optional<int>") {
    optional<int> x(1);
    REQUIRE(x == true);
    REQUIRE(*x == 1);
    THEN("Assigne none") {
      x = optional<int>();
      REQUIRE(x == false);
    }
  }
  WHEN("Copy-constructing") {
    optional<int> x(1);
    optional<int> y(x);
    REQUIRE(*x == 1);
    REQUIRE(*y == 1);
  }
  WHEN("Assigning optionals to each other") {
    optional<int> x;
    REQUIRE(x == false);
    optional<int> y(1);
    x = y;
    REQUIRE(*x == 1);
    REQUIRE(*y == 1);
    REQUIRE(*x == *y);
  }
}

SCENARIO("Test entity properties") {
  using namespace optspp;
  auto n1 = named(name("first"));
  REQUIRE(n1->kind() == scheme::entity::KIND::ARGUMENT);
  REQUIRE(n1->long_names());
  REQUIRE((*n1->long_names()).size() == 1);
  auto v1 = value("yes");
  REQUIRE(v1->known_values());
  REQUIRE(*v1->known_values() == std::vector<std::string>({"yes"}));
  auto n2 = named(name("second"),
                  implicit_values("implicit"));
  REQUIRE(n2->implicit_values());
  REQUIRE((*n2->implicit_values())[0] == "implicit");
}

SCENARIO("XOR scheme without 'any' values and positional args") {
  using namespace optspp;
  WHEN("Created the scheme") {
    scheme::definition args;
    args
      << (named(name("Arg_L1_1"))
          << (value("Val_L2_1")
              << (named(name("Arg_L3_1"))
                  << (value("Val_L4_1"))
                  << (value("Val_L4_2")))
              << (named(name("Arg_L3_2"))))
          << (value("Val_L2_2"))
          )
      << (named(name("Arg_L1_2"))
          << (value("Val_L2_2_1"))
          << (value("Val_L2_2_2")));
    REQUIRE(args.root()->children().size() == 2);
    auto& arg_l1_1 = args.root()->children()[0];
    auto& val_l2_1 = arg_l1_1->children()[0];
    auto& arg_l3_1 = val_l2_1->children()[0];
    auto& val_l4_1 = arg_l3_1->children()[0];
    auto& val_l4_2 = arg_l3_1->children()[1];
    auto& arg_l3_2 = val_l2_1->children()[1];
    auto& val_l2_2 = arg_l1_1->children()[1];
    auto& arg_l1_2 = args.root()->children()[1];


    REQUIRE(arg_l1_1->long_names());
    REQUIRE((*arg_l1_1->long_names())[0] == "Arg_L1_1");
    REQUIRE(arg_l1_1->siblings_group() == scheme::SIBLINGS_GROUP::XOR);
    REQUIRE(arg_l1_2->long_names());
    REQUIRE((*arg_l1_2->long_names())[0] == "Arg_L1_2");
    REQUIRE(arg_l1_2->siblings_group() == scheme::SIBLINGS_GROUP::XOR);
    REQUIRE(val_l2_1->known_values());
    REQUIRE((*val_l2_1->known_values())[0] == "Val_L2_1");
    REQUIRE(val_l2_2->known_values());
    REQUIRE((*val_l2_2->known_values())[0] == "Val_L2_2");
    
    THEN("Command line: --Arg_L1_1 Val_L2_1 --Arg_L3_1 Val_L4_2") {
      scheme::parser p(args, {"--Arg_L1_1", "Val_L2_1",  "--Arg_L3_1", "Val_L4_2"});
      p.initialize_pass();
      auto parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(p.consume_argument(parent));
      //REQUIRE(arg_l1_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l1_2->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l2_1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(val_l2_2->color() == scheme::entity::COLOR::BLOCKED);
      parent = p.find_border_entity();
      REQUIRE(parent == val_l2_1);
      REQUIRE(p.consume_argument(parent));
      // REQUIRE(arg_l3_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l3_2->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l4_1->color() == scheme::entity::COLOR::BLOCKED);
      REQUIRE(val_l4_2->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(p.find_border_entity() == nullptr);
      THEN("Check results") {
        REQUIRE(args["Arg_L1_1"].size() == 1);
        REQUIRE(args["Arg_L1_1"][0] == "Val_L2_1");
        REQUIRE(args["Arg_L3_1"].size() == 1);
        REQUIRE(args["Arg_L3_1"][0] == "Val_L4_2");
        REQUIRE(args["Arg_L3_2"].size() == 0);
        REQUIRE(args["Arg_L1_2"].size() == 0);
      }
    }
  }            
}

SCENARIO("XOR/OR scheme with 'any' values and positional args") {
  using namespace optspp;
  WHEN("Created the scheme") {
    scheme::definition args;
    args
      | (named(name("Arg_L1_1"))
          << (value("Val_1_L2_1")
              << (named(name("Arg_L3_1"))
                  | (value("Val_3_L4_1"))
                  | (value("Val_3_L4_2")))
              << (named(name("Arg_L3_2"),
                        name('a'))
                  | (value("Val_3_2_L4_1"))
                  | (value(any()))
                  ))
         << (value("Val_1_L2_2"))
         )
      | (named(name("Arg_L1_2"))
         << (value("Val_2_L2_1"))
         << (value("Val_2_L2_2")))
      | (positional(name("positional"))
         << (value("pos_val1"))
         << (value("pos_val2")));
    
    REQUIRE(args.root()->children().size() == 3);
    auto& arg_l1_1 = args.root()->children()[0];
    auto& val_1_l2_1 = arg_l1_1->children()[0];
    auto& arg_l3_1 = val_1_l2_1->children()[0];
    auto& val_3_l4_1 = arg_l3_1->children()[0];
    auto& val_3_l4_2 = arg_l3_1->children()[1];
    auto& arg_l3_2 = val_1_l2_1->children()[1];
    auto& val_3_2_l4_1 = arg_l3_2->children()[0];
    auto& val_3_2_l4_any = arg_l3_2->children()[1];
    auto& val_1_l2_2 = arg_l1_1->children()[1];
    auto& arg_l1_2 = args.root()->children()[1];
    auto& val_2_l2_1 = arg_l1_2->children()[0];
    auto& val_2_l2_2 = arg_l1_2->children()[1];

    auto& positional = args.root()->children()[2];
    auto& positional_val1 = positional->children()[0];
    auto& positional_val2 = positional->children()[1];

    REQUIRE(arg_l1_1->long_names());
    REQUIRE((*arg_l1_1->long_names())[0] == "Arg_L1_1");
    REQUIRE(arg_l1_1->siblings_group() == scheme::SIBLINGS_GROUP::OR);
    REQUIRE(arg_l1_2->long_names());
    REQUIRE((*arg_l1_2->long_names())[0] == "Arg_L1_2");
    REQUIRE(arg_l1_2->siblings_group() == scheme::SIBLINGS_GROUP::OR);
    REQUIRE(val_1_l2_1->known_values());
    REQUIRE((*val_1_l2_1->known_values())[0] == "Val_1_L2_1");
    REQUIRE(val_1_l2_2->known_values());
    REQUIRE((*val_1_l2_2->known_values())[0] == "Val_1_L2_2");
    REQUIRE(val_3_2_l4_1->known_values());
    REQUIRE((*val_3_2_l4_1->known_values())[0] == "Val_3_2_L4_1");

    std::vector<std::string> input{"--Arg_L1_1", "Val_1_L2_1",  "--Arg_L3_1", "Val_3_L4_2", "--Arg_L3_1", "Val_3_L4_1", "--Arg_L1_2", "Val_2_L2_2", "pos_val1" };
    THEN("Step by step") {
      scheme::parser p(args, input);
      // Cycle 1
      p.initialize_pass();
      auto parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(p.consume_argument(parent));
      //REQUIRE(arg_l1_1->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(arg_l1_2->color() == scheme::entity::COLOR::NONE);
      REQUIRE(val_1_l2_1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(val_1_l2_2->color() == scheme::entity::COLOR::BLOCKED);
      parent = p.find_border_entity();
      REQUIRE(parent == val_1_l2_1);
      REQUIRE(p.consume_argument(parent));
      REQUIRE(arg_l3_1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(val_3_l4_2->color() == scheme::entity::COLOR::BORDER);
      parent = p.find_border_entity();
      REQUIRE(parent == nullptr);

      // Cycle 2
      p.initialize_pass();
      parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(p.consume_argument(parent));
      //REQUIRE(arg_l1_2->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(val_2_l2_2->color() == scheme::entity::COLOR::BORDER);
      parent = p.find_border_entity();
      REQUIRE(parent == val_1_l2_1);
      REQUIRE(p.consume_argument(parent));
      REQUIRE(arg_l3_1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(val_3_l4_1->color() == scheme::entity::COLOR::BORDER);
      parent = p.find_border_entity();
      REQUIRE(parent == nullptr);

      // Cycle 3
      p.initialize_pass();
      parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(p.consume_argument(parent));
      //REQUIRE(positional->color() == scheme::entity::COLOR::VISITED);
      REQUIRE(positional_val1->color() == scheme::entity::COLOR::BORDER);
      REQUIRE(positional_val2->color() == scheme::entity::COLOR::BLOCKED);
      parent = p.find_border_entity();
      REQUIRE(parent == val_1_l2_1);
      REQUIRE(!p.consume_argument(parent));

      // Cycle 4
      p.initialize_pass();
      parent = p.find_border_entity();
      REQUIRE(parent == args.root());
      REQUIRE(!p.consume_argument(parent));
      parent = p.find_border_entity();
      REQUIRE(parent == val_1_l2_1);
      REQUIRE(!p.consume_argument(parent));
      parent = p.find_border_entity();
      REQUIRE(parent == nullptr);

      THEN("Check resulting arg values") {
        REQUIRE(args["Arg_L1_1"].size() == 1);
        REQUIRE(args["Arg_L1_1"][0] == "Val_1_L2_1");
        REQUIRE(args["Arg_L3_1"].size() == 2);
        REQUIRE(args["Arg_L3_1"][0] == "Val_3_L4_2");
        REQUIRE(args["Arg_L3_1"][1] == "Val_3_L4_1");
        REQUIRE(args["Arg_L1_2"].size() == 1);
        REQUIRE(args["Arg_L1_2"][0] == "Val_2_L2_2");
        REQUIRE(args["positional"].size() == 1);
        REQUIRE(args["positional"][0] == "pos_val1");
      }
    }
    THEN("High-level") {
      args.parse(input);
      REQUIRE(args["Arg_L1_1"].size() == 1);
      REQUIRE(args["Arg_L1_1"][0] == "Val_1_L2_1");
      REQUIRE(args["Arg_L3_1"].size() == 2);
      REQUIRE(args["Arg_L3_1"][0] == "Val_3_L4_2");
      REQUIRE(args["Arg_L3_1"][1] == "Val_3_L4_1");
      REQUIRE(args["Arg_L1_2"].size() == 1);
      REQUIRE(args["Arg_L1_2"][0] == "Val_2_L2_2");
      REQUIRE(args["positional"].size() == 1);
      REQUIRE(args["positional"][0] == "pos_val1");
    }
  }            
}


SCENARIO("Throw when unparsed args are left") {
  using namespace optspp;
  const std::vector<std::string> input{"--first", "yes", "--second", "no"};
  scheme::definition args1;
  args1 << (named(name("first"))
            << (value("yes"))
            << (value("no")))
        << (named(name("second"))
            << (value("yes"))
            << (value("no")));
  REQUIRE_THROWS(args1.parse(input));
    
  scheme::definition args2;
  args2 | (named(name("first"))
           << (value("yes"))
           << (value("no")))
    | (named(name("second"))
       << (value("yes"))
       << (value("no")));
  REQUIRE_NOTHROW(args2.parse(input));
}

SCENARIO("Realistic config") {
  WHEN("Given a realistic arguments scheme") {
    using namespace optspp;
    auto option_force =
      named(name("force"),
            name('f'),
            default_values("false"),
            implicit_values("true"))
      << value("true", {"on", "yes"})
      << value("false", {"off", "no"});

    
    scheme::definition arguments;
    arguments <<
      (positional(name("command"))
       << (value("useradd")
           | (named(name("login", {"username", "user"}),
                    name('l', {'u'}),
                    min_count(1),
                    max_count(1),
                    description("User's login")))
           | (named(name("password", {"pw", "pass"}),
                    name('p'),
                    max_count(1),
                    description("User's password")))
           | (named(name("admin", {"administrator"}),
                    name('a'),
                    description("Make this user administrator"))
              | (value("true", {"on", "yes"})
                 << (named(name("super-admin"),
                           default_values("false"),
                           implicit_values("true"),
                           description("Make this administrator a superadministrator"))
                     << value("true", {"on", "yes"})
                     << value("false", {"off", "no"}))
                 << (named(name("rookie-admin"),
                           default_values("true"),
                           implicit_values("true"),
                           description("Make this administrator a rookie administrator"))
                     << value("true", {"on", "yes"})
                     << value("false", {"off", "no"})))
              | (value("false", {"off", "no"})))
           | (option_force))
       << (value("userdel")
           | (named(name("login", {"username", "user"}),
                    name('l', {'u'}),
                    min_count(1),
                    max_count(1),
                    description("User's login")))
           | (option_force))
       << (value(any())));

    THEN("useradd --super-admin --admin yes --login mylogin") {
      std::vector<std::string> arguments_input{"useradd", "--super-admin", "--admin", "yes", "--login", "mylogin"};
      arguments.parse(arguments_input);
    }
    THEN("useradd --super-admin --admin yes --login mylogin -p secret") {
      std::vector<std::string> arguments_input{"useradd", "--super-admin", "--admin", "yes", "--login", "mylogin", "-p", "secret"};
      arguments.parse(arguments_input);
    }
    THEN("userdel --force --login mylogin") {
      std::vector<std::string> arguments_input{"userdel", "--force", "--login", "mylogin"};
      arguments.parse(arguments_input);
      REQUIRE(arguments["force"].size() == 1);
      REQUIRE(arguments["force"][0] == "true");
    }
  }
}

SCENARIO("rm example") {
  using namespace optspp;
  scheme::definition arguments;

  WHEN("Somplified scheme is given") {
    arguments
      // Declare a named argument node
      | (named(name("force"),  // Set long name attribute to "force"
               name('f'),      // Set short name attribute to 'f'
               default_values("false"),   // Which value to assume if not given on the cmd line
               implicit_values("true"),   // Which value to assume if given on cmd line without value
               max_count(1))    // Maximum number of times the argument may be specified
         << value("true", {"on", "yes"})  // Possible value with synonyms
         << value("false", {"off", "no"}))
      | (named(name("recursive"),
               name('r', {'R'}),  // Short name with one synonym
               default_values("false"),
               implicit_values("true"))
         << value("true", {"on", "yes"})
         << value("false", {"off", "no"}))
      | (positional(name("filename"),
                    min_count(1))
         << (value(any())));

    THEN("-rf file1 file2") {
      std::vector<std::string> input{"-rf", "file1", "file2"};
      REQUIRE_NOTHROW(arguments.parse(input));
      REQUIRE(arguments["force"].size() == 1);
      REQUIRE(arguments["force"][0] == "true");
      REQUIRE(arguments["recursive"].size() == 1);
      REQUIRE(arguments["recursive"][0] == "true");
      REQUIRE(arguments["filename"].size() == 2);
      REQUIRE(arguments["filename"][0] == "file1");
      REQUIRE(arguments["filename"][1] == "file2");
    }

    THEN("-rf on file1 file2") {
      std::vector<std::string> input{"-rf", "on", "file1", "file2"};
      REQUIRE_NOTHROW(arguments.parse(input));
      REQUIRE(arguments["force"].size() == 1);
      REQUIRE(arguments["force"][0] == "true");
      REQUIRE(arguments["recursive"].size() == 1);
      REQUIRE(arguments["recursive"][0] == "true");
      REQUIRE(arguments["filename"].size() == 2);
      REQUIRE(arguments["filename"][0] == "file1");
      REQUIRE(arguments["filename"][1] == "file2");
    }

    THEN("-r -f file1 file2") {
      std::vector<std::string> input{"-r", "-f", "file1", "file2"};
      REQUIRE_NOTHROW(arguments.parse(input));
      REQUIRE(arguments["force"].size() == 1);
      REQUIRE(arguments["force"][0] == "true");
      REQUIRE(arguments["recursive"].size() == 1);
      REQUIRE(arguments["recursive"][0] == "true");
      REQUIRE(arguments["filename"].size() == 2);
      REQUIRE(arguments["filename"][0] == "file1");
      REQUIRE(arguments["filename"][1] == "file2");
    }
    
    THEN("-R --force file1 file2") {
      std::vector<std::string> input{"-R", "--force", "file1", "file2"};
      REQUIRE_NOTHROW(arguments.parse(input));
      REQUIRE(arguments["force"].size() == 1);
      REQUIRE(arguments["force"][0] == "true");
      REQUIRE(arguments["recursive"].size() == 1);
      REQUIRE(arguments["recursive"][0] == "true");
      REQUIRE(arguments["filename"].size() == 2);
      REQUIRE(arguments["filename"][0] == "file1");
      REQUIRE(arguments["filename"][1] == "file2");
    }

    THEN("-f true -f true file1") {
      std::vector<std::string> input{"-f", "true", "-f", "true", "file1"};
      REQUIRE_THROWS_AS(arguments.parse(input), actual_counts_mismatch);
    }

    THEN("-rf") {
      std::vector<std::string> input{"-rf"};
      REQUIRE_THROWS_AS(arguments.parse(input), actual_counts_mismatch);
    }
    
    THEN("--force yes --force no file1 file2") {
      std::vector<std::string> input{"-R", "--force", "yes", "--force", "no", "file1", "file2"};
      REQUIRE_THROWS_AS(arguments.parse(input), value_conflict);
    }
  }
}
