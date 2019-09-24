/* Unit testing for outcomes
(C) 2013-2019 Niall Douglas <http://www.nedproductions.biz/> (8 commits)


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
    (See accompanying file Licence.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

#include "../../include/outcome/experimental/status_result.hpp"

template <class T, class S = SYSTEM_ERROR2_NAMESPACE::system_code, class NoValuePolicy = OUTCOME_V2_NAMESPACE::experimental::policy::default_status_result_policy<T, S>> using result = OUTCOME_V2_NAMESPACE::experimental::status_result<T, S, NoValuePolicy>;
using OUTCOME_V2_NAMESPACE::in_place_type;

#include "quickcpplib/boost/test/unit_test.hpp"

#include <exception>
#include <iostream>

#ifdef __cpp_exceptions
// Custom error type with payload
struct payload
{
  SYSTEM_ERROR2_NAMESPACE::errc ec{SYSTEM_ERROR2_NAMESPACE::errc::success};
  const char *str{nullptr};
  payload() = default;
  payload(SYSTEM_ERROR2_NAMESPACE::errc _ec, const char *_str)
      : ec(_ec)
      , str(_str)
  {
  }
};
struct payload_exception : std::exception
{
  const char *_what{nullptr};
  explicit payload_exception(const char *what)
      : _what(what)
  {
  }
  virtual const char *what() const noexcept override final { return _what; }  // NOLINT
};

class _payload_domain;
using status_code_payload = SYSTEM_ERROR2_NAMESPACE::status_code<_payload_domain>;
class _payload_domain : public SYSTEM_ERROR2_NAMESPACE::status_code_domain
{
  template <class> friend class status_code;
  using _base = SYSTEM_ERROR2_NAMESPACE::status_code_domain;

public:
  using value_type = payload;
  using string_ref = _base::string_ref;

public:
  constexpr _payload_domain() noexcept : _base(0x7b782c8f935e34ba) {}

  static inline constexpr const _payload_domain &get();

  virtual _base::string_ref name() const noexcept override final { return string_ref("payload domain"); }  // NOLINT
protected:
  virtual bool _do_failure(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);                                                                              // NOLINT
    return static_cast<const status_code_payload &>(code).value().ec != SYSTEM_ERROR2_NAMESPACE::errc::success;  // NOLINT
  }
  virtual bool _do_equivalent(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code1, const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code2) const noexcept override final  // NOLINT
  {
    assert(code1.domain() == *this);                                   // NOLINT
    const auto &c1 = static_cast<const status_code_payload &>(code1);  // NOLINT
    if(code2.domain() == *this)
    {
      const auto &c2 = static_cast<const status_code_payload &>(code2);  // NOLINT
      return c1.value().ec == c2.value().ec;
    }
    return false;
  }
  virtual SYSTEM_ERROR2_NAMESPACE::generic_code _generic_code(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);                                    // NOLINT
    return static_cast<const status_code_payload &>(code).value().ec;  // NOLINT
  }
  virtual _base::string_ref _do_message(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const noexcept override final  // NOLINT
  {
    assert(code.domain() == *this);                                  // NOLINT
    const auto &c = static_cast<const status_code_payload &>(code);  // NOLINT
    static SYSTEM_ERROR2_CONSTEXPR14 SYSTEM_ERROR2_NAMESPACE::detail::generic_code_messages msgs;
    return string_ref(msgs[static_cast<int>(c.value().ec)]);
  }
  virtual void _do_throw_exception(const SYSTEM_ERROR2_NAMESPACE::status_code<void> &code) const override final  // NOLINT
  {
    assert(code.domain() == *this);                                  // NOLINT
    const auto &c = static_cast<const status_code_payload &>(code);  // NOLINT
    throw payload_exception(c.value().str);
  }
};
constexpr _payload_domain payload_domain;
inline constexpr const _payload_domain &_payload_domain::get()
{
  return payload_domain;
}
inline status_code_payload make_status_code(payload c) noexcept
{
  return status_code_payload(SYSTEM_ERROR2_NAMESPACE::in_place, c);
}
#endif

BOOST_OUTCOME_AUTO_TEST_CASE(works / status_code / result, "Tests that the result with status_code works as intended")
{
  using namespace SYSTEM_ERROR2_NAMESPACE;

  {  // errored int
    result<int> m(generic_code{errc::bad_address});
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), generic_error);
    BOOST_CHECK_NO_THROW(m.error());
  }
  {  // errored void
    result<void> m(generic_code{errc::bad_address});
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(([&m]() -> void { return m.value(); }()), generic_error);
    BOOST_CHECK_NO_THROW(m.error());
  }
  {  // valued int
    result<int> m(5);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == 5);
    m.value() = 6;
    BOOST_CHECK(m.value() == 6);
  }
  {  // valued bool
    result<bool> m(false);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == false);
    m.value() = true;
    BOOST_CHECK(m.value() == true);
  }
  {  // moves do not clear state
    result<std::string> m("niall");
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == "niall");
    m.value() = "NIALL";
    BOOST_CHECK(m.value() == "NIALL");
    auto temp(std::move(m).value());
    BOOST_CHECK(temp == "NIALL");
    BOOST_CHECK(m.value().empty());  // NOLINT
  }
  {  // valued void
    result<void> m(in_place_type<void>);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_NO_THROW(m.value());  // works, but type returned is unusable
  }
  {  // errored
    error ec(errc::no_link);
    result<int> m(ec.clone());
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), generic_error);
    BOOST_CHECK(m.error() == ec);
  }
  if(false)  // NOLINT
  {          // void, void is permitted, but is not constructible
    result<void, void> *m = nullptr;
    m->value();
    m->error();
  }

  {
    // Deliberately define non-trivial operations
    struct udt
    {
      int _v{0};
      udt() = default;
      udt(udt &&o) noexcept : _v(o._v) {}
      udt(const udt &o)  // NOLINT
      : _v(o._v)
      {
      }
      udt &operator=(udt &&o) noexcept
      {
        _v = o._v;
        return *this;
      }
      udt &operator=(const udt &o)  // NOLINT
      {
        _v = o._v;
        return *this;
      }
      ~udt() { _v = 0; }
    };
    // No default construction, no copy nor move
    struct udt2
    {
      udt2() = delete;
      udt2(udt2 &&) = delete;
      udt2(const udt2 &) = delete;
      udt2 &operator=(udt2 &&) = delete;
      udt2 &operator=(const udt2 &) = delete;
      explicit udt2(int /*unused*/) {}
      ~udt2() = default;
    };
    // Can only be constructed via multiple args
    struct udt3
    {
      udt3() = delete;
      udt3(udt3 &&) = delete;
      udt3(const udt3 &) = delete;
      udt3 &operator=(udt3 &&) = delete;
      udt3 &operator=(const udt3 &) = delete;
      explicit udt3(int /*unused*/, const char * /*unused*/, std::nullptr_t /*unused*/) {}
      ~udt3() = default;
    };

    result<int> a(5);
    result<int> b(generic_code{errc::invalid_argument});
    std::cout << sizeof(a) << std::endl;  // 32 bytes
    if(false)                             // NOLINT
    {
      b.assume_value();
      a.assume_error();
    }
#ifdef __cpp_exceptions
    try
    {
      b.value();
      std::cerr << "fail" << std::endl;
      std::terminate();
    }
    catch(const generic_error &e)
    {
      BOOST_CHECK(!strcmp(e.what(), b.error().message().c_str()));
    }
#endif
    static_assert(!std::is_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(a)>::value, "");
// Quality of implementation of std::optional is poor :(
#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
    static_assert(!std::is_trivially_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(a)>::value, "");
#endif
    static_assert(!std::is_trivially_destructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(a)>::value, "");

    // Test void compiles
    result<void> c(in_place_type<void>);

    // Test a standard udt compiles
    result<udt> d(in_place_type<udt>);
    static_assert(!std::is_default_constructible<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(d)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(d)>::value, "");
    static_assert(std::is_move_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_move_assignable<decltype(d)>::value, "");
    static_assert(std::is_nothrow_move_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_destructible<decltype(d)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(d)>::value, "");

    // Test a highly pathological udt compiles
    result<udt2> e(in_place_type<udt2>, 5);
    // result<udt2> e2(e);
    static_assert(!std::is_default_constructible<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(e)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_move_assignable<decltype(e)>::value, "");
    static_assert(!std::is_trivially_move_assignable<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_move_assignable<decltype(e)>::value, "");

    // Test a udt which can only be constructed in place compiles
    result<udt3> g(in_place_type<udt3>, 5, static_cast<const char *>("niall"), nullptr);
    // Does converting inplace construction also work?
    result<udt3> h(5, static_cast<const char *>("niall"), nullptr);
    result<udt3> i(generic_code{errc::not_enough_memory});
    BOOST_CHECK(h.has_value());
    BOOST_CHECK(i.has_error());
  }

  // Test direct use of error code enum works
  {
    constexpr result<int, errc, OUTCOME_V2_NAMESPACE::policy::all_narrow> a(5), b(errc::invalid_argument);
    static_assert(a.value() == 5, "a is not 5");
    static_assert(b.error() == errc::invalid_argument, "b is not errored");
  }

#ifdef __cpp_exceptions
  // Test payload facility
  {
    const char *niall = "niall";
    result<int, status_code_payload> b{payload{errc::invalid_argument, niall}};
    try
    {
      b.value();
      BOOST_CHECK(false);
    }
    catch(const payload_exception &e)
    {
      BOOST_CHECK(!strcmp(e.what(), niall));
    }
    catch(...)
    {
      BOOST_CHECK(false);
    }
  }
#endif
}
