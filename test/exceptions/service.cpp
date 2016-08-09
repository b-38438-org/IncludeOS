// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <os>
#include <stdio.h>
#include <cassert>
#include <lest.hpp>

#define MYINFO(X,...) INFO("Test exceptions",X,##__VA_ARGS__)

// FIXME: Remove hack below :-(
int clock_gettime(clockid_t clk_id, struct timespec *tp){
  // Lest depends on clock_gettime, but it's not implemented in
  // IncludeOS yet. Add a stub to prevent linker error.
  (void*)clk_id;
  (void*)tp;
  return 0;
};

const lest::test tests[] = {
  {
    SCENARIO("exceptions can be thrown and caught") {
      GIVEN ("a custom exception class") {
        class CustomException : public std::runtime_error {
          using runtime_error::runtime_error;
        };

        THEN("a runtime exception should be caught") {
          const char *error_msg = "Crazy Error!";

          bool caught = false;
          try {
            if (OS::uptime() > 0.1){ // TODO: Why uptime? Ask Alfred.
              std::runtime_error myexception(error_msg);
              throw myexception;
            }
          } catch(std::runtime_error e) {
            caught = std::string(e.what()) == std::string(error_msg);
          }
          EXPECT(caught);
        }

        THEN("a custom exception should be caught") {
          std::string custom_msg = "Custom exceptions are useful";
          std::string caught_msg = "";

          try {
            throw CustomException(custom_msg);
          } catch (CustomException e){
            caught_msg = e.what();
          }

          EXPECT(caught_msg == custom_msg);
        };
      }
    }
  },
};

void Service::start()
{
  MYINFO ("Running LEST-tests");
  auto failed = lest::run(tests, {"-p"});
  assert(not failed);
  MYINFO("SUCCESS");
}