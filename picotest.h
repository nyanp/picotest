/*
    Copyright (c) 2012, Taiga Nomi
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	* Neither the name of the <organization> nor the
	names of its contributors may be used to endorse or promote products
	derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY 
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <sstream>
#include <algorithm>

// workaround
#if defined _WIN32
#define PICOTEST_WINDOWS
#include <Windows.h>
#endif


namespace picotest {

/////////////////////////////////////////////////////////////////
// forward decralations

class  TestCase;
class  Test;
struct TestState;
struct Registry;
struct Registrar;

namespace detail {
	enum Color {
		COLOR_RED,
		COLOR_GREEN
	};
	TestCase* getCurrentTestCase();
	Test* getCurrentTest();
	void setCurrentTestCase(TestCase* testcase);
	void setCurrentTest(Test* test);
	void coloredPrint(Color c, const char* fmt, ...);

	template <typename Char, typename CharTraits, typename T>
	::std::basic_ostream<Char, CharTraits>& operator<<(
		::std::basic_ostream<Char, CharTraits>& os, const T& v) {
		os << "(" << sizeof(v) << "-byte object)";
		return os;
	}
}

/////////////////////////////////////////////////////////////////

struct TestState {
	TestState() : testcase_(0), test_(0) {}

	static TestState& getInstance() {
		static TestState instance;
		return instance;
	}
	TestCase* testcase_;
	Test* test_;
};

struct Failure {
	Failure(const std::string& file, int line, const std::string& expected, const std::string& actual)
		: file(file), line(line), message("Expected:" + expected + ", Actual:" + actual) {}

	Failure() {}
	std::string file;
	int line;
	std::string message;
};

class Test {
public:
	typedef void (*TestFunc)(void);

	Test (const std::string& name, TestFunc f) : executed_(false), name_(name), f_(f) {}

	void execute() {
		detail::setCurrentTest(this);
		f_();
		executed_ = true;
	}

	const std::string& name() const {
		return name_;
	}

	bool success() const {
		return executed_ && failures_.empty();
	}

	void setFailure(const std::string& file, int line, const std::string& expected, const std::string& result) {
		failures_.push_back(Failure(file, line, expected, result));
	}

	void reportFailure(std::ostream& os) const {
		for (std::vector<Failure>::const_iterator it = failures_.begin(), end = failures_.end(); it != end; ++it) 
			report(os, (*it));
	}
private:
	void report(std::ostream& os, const Failure& f) const {
		os << name_ << " : " << f.file << "(" << f.line << "): error: " << f.message << std::endl;
	}

	bool executed_;
	std::vector<Failure> failures_;
	std::string name_;
	TestFunc f_;
};

class TestCase {
public:
	TestCase() {}
	TestCase(const std::string& name) : executed_(false), name_(name) {}

	void add(const Test&t) {
		tests_.push_back(t);
	}

	void execute() {
		detail::setCurrentTestCase(this);
		
		std::for_each(tests_.begin(), tests_.end(), std::mem_fun_ref(&Test::execute));

		executed_ = true;
	}

	void report(std::ostream& os) const {
		coloredPrint(detail::COLOR_RED, "[ FAILED ] ");
		os << name_ << std::endl;

		for (std::vector<Test>::const_iterator it = tests_.begin(), end = tests_.end(); it != end; ++it)
			if (!(*it).success()) (*it).reportFailure(os);
	}

	bool success() const {
		if (!executed_) return false;

		if (std::find_if(tests_.begin(), tests_.end(), std::not1(std::mem_fun_ref(&Test::success))) == tests_.end())
			return true;

		return false; // at least one test failed
	}

	const std::string& name() const {
		return name_;
	}

private:
	bool executed_;
	std::string name_;
	std::vector<Test> tests_;
};

struct Registry {
public:
	static Registry& getInstance() {
		static Registry instance;
		return instance;
	}

	void add(const std::string& test_case_name, const Test& t) {
		std::vector<TestCase>::iterator found = find_by_name(test_case_name);

		if (found == tests_.end()) {
			tests_.push_back(TestCase(test_case_name));
			found = find_by_name(test_case_name);
		}
		(*found).add(t);
	}

	void testRun() {
		std::for_each(tests_.begin(), tests_.end(), std::mem_fun_ref(&TestCase::execute));
	}

	void report(std::ostream& os) const {
		int failed = numFailed();

		if (failed) {
			os << numFailed() << " of " << numTotal() << " tests failed." << std::endl;

			for (std::vector<TestCase>::const_iterator it = tests_.begin(), end = tests_.end(); it != end; ++it)
				if (!(*it).success()) (*it).report(os);
		} else {
			os << numSuccess() << "tests success." << std::endl;
		}
	}

	bool fail() const {
		return numTotal() > 0 && numFailed() > 0;
	}

	int numFailed() const {
		return numTotal() - numSuccess();
	}

	int numSuccess() const {
		return std::count_if(tests_.begin(), tests_.end(), std::mem_fun_ref(&TestCase::success));
	}

	int numTotal() const {
		return tests_.size();
	}

private:
	std::vector<TestCase>::iterator find_by_name(const std::string& test_case_name) {
		for (std::vector<TestCase>::iterator it = tests_.begin(), end = tests_.end(); it != end; ++it)
			if ((*it).name() == test_case_name) return it;

		return tests_.end();
	}

	std::vector<TestCase> tests_;

};

struct Registrar {
	Registrar (const std::string& test_case_name, const Test& t) {
		Registry::getInstance().add(test_case_name, t);
	}
};

namespace detail {

	TestCase* getCurrentTestCase() {
		return TestState::getInstance().testcase_;
	}

	Test* getCurrentTest() {
		return TestState::getInstance().test_;
	}

	void setCurrentTestCase(TestCase* testcase) {
		TestState::getInstance().testcase_ = testcase;
	}

	void setCurrentTest(Test* test) {
		TestState::getInstance().test_ = test;
	}

	bool assertBool(bool expected, bool result, const std::string& cond, const std::string& file, int line) {
		if (expected != result) {
			Test* current = getCurrentTest();
			assert(current);
			current->setFailure(file, line, cond, result ? "true" : "false");
		}
		return expected == result;
	}

	template<typename T1, typename T2, typename OP>
	bool assertBinary(const T1& expected, const T2& result, OP op, const std::string& cond, const std::string& file, int line) {

		bool c = op(expected, result);

		if (!c) {
			Test* current = getCurrentTest();
			assert(current);
			current->setFailure(file, line, cond, ::foo::condStr(expected, result, op));
		}
		return c;
	}


#ifdef PICOTEST_WINDOWS
	WORD getColorAttr(Color c) {
		switch (c) {
		case COLOR_RED:    return FOREGROUND_RED;
		case COLOR_GREEN:  return FOREGROUND_GREEN;
		default:           assert(0); return 0;
		}
	}
#endif

	void coloredPrint(Color c, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

#ifdef PICOTEST_WINDOWS
		const HANDLE std_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO buffer_info;
		::GetConsoleScreenBufferInfo(std_handle, &buffer_info);
		const WORD old_color = buffer_info.wAttributes;
		const WORD new_color  = getColorAttr(c) | FOREGROUND_INTENSITY;

		fflush(stdout);
		::SetConsoleTextAttribute(std_handle, new_color);

		vprintf(fmt, args);

		fflush(stdout);
		::SetConsoleTextAttribute(std_handle, old_color);
#else
		vprintf(fmt, args);
#endif
		va_end(args);
	}

	struct yes {};
	struct no {};
	
	template<bool B>
	struct is_true {
		typedef yes type;
	};

	template<>
	struct is_true<false>{
		typedef no type;
	};

	template<typename T>
	struct has_insertion_op {
		typedef char yes;
		typedef struct { char a[2]; } no;
		template<typename U> static yes& test(U&);
		template<typename U> static no& test(...);
		static std::ostream& s;
		static const T& t;
		static const bool value = sizeof(test(s << t)) == sizeof(yes);
	};

	/*template<bool B, class T = void>
	struct enable_if_c {
		typedef T type;
	};

	template<class T>
	struct enable_if_c<false, T> {};

	template<typename Cond, class T = void>
	struct enable_if : public enable_if_c<Cond::value, T> {};*/





	// binary operators
	struct LT {
		template <class T1, class T2>
		bool operator()(const T1& v1, const T2& v2) { return v1 < v2; }
		static std::string toStr() { return "<"; }
	};

	struct GT {
		template <class T1, class T2>
		bool operator()(const T1& v1, const T2& v2) { return v1 > v2; }
		static std::string toStr() { return ">"; }
	};

	struct LE {
		template <class T1, class T2>
		bool operator()(const T1& v1, const T2& v2) { return v1 <= v2; }
		static std::string toStr() { return "<="; }
	};

	struct GE {
		template <class T1, class T2>
		bool operator()(const T1& v1, const T2& v2) { return v1 >= v2; }
		static std::string toStr() { return ">="; }
	};

	struct EQ {
		template <class T1, class T2>
		bool operator()(const T1& v1, const T2& v2) { return v1 == v2; }
		static std::string toStr() { return "=="; }
	};

	struct NE {
		template <class T1, class T2>
		bool operator()(const T1& v1, const T2& v2) { return v1 == v2; }
		static std::string toStr() { return "!="; }
	};

	struct STREQ {
		bool operator()(const char* v1, const char* v2) { return strcmp(v1, v2) == 0; }
		static std::string toStr() { return "=="; }
	};

	struct STRNE {
		bool operator()(const char* v1, const char* v2) { return strcmp(v1, v2) != 0; }
		static std::string toStr() { return "!="; }
	};

	struct STRCASEEQ {
		bool operator()(const char* v1, const char* v2) { return _stricmp(v1, v2) == 0; }
		static std::string toStr() { return "=="; }
	};

	struct STRCASENE {
		bool operator()(const char* v1, const char* v2) { return _stricmp(v1, v2) != 0; }
		static std::string toStr() { return "!="; }
	};

} // namespace detail

} // namespace picotest

namespace testing {

class Test {
public:
	Test() {}
	virtual ~Test() {}
	void execute() {
		SetUp();
		test_method(); // template-method
		TearDown();
	}
protected:
	virtual void SetUp() {}
	virtual void TearDown() {}
	virtual void test_method() = 0;
};

} // namespace testing

namespace foo {
	template<typename T1, typename T2, typename OP>
	std::string condStr(const T1& v1, const T2& v2, OP op){
		std::ostringstream os;
		using namespace ::picotest::detail;

		os << v1 << " " << op.toStr() << " " << v2;
		return os.str();
	}
}

/////////////////////////////////////////////////////////////////
// helper macros

#define PICOTEST_JOIN(X, Y)       PICOTEST_DO_JOIN( X, Y )
#define PICOTEST_DO_JOIN( X, Y )  PICOTEST_DO_JOIN2(X,Y)
#define PICOTEST_DO_JOIN2( X, Y ) X##Y
#define PICOTEST_STR(X) #X

#define PICOTEST_IDENITY(test_case_name, test_name) PICOTEST_JOIN(test_case_name, test_name)
#define PICOTEST_TEST_CASE_INVOKER(test_case_name, test_name) PICOTEST_JOIN(PICOTEST_IDENITY(test_case_name, test_name), _invoker)
#define PICOTEST_TEST_CASE_REGISTRAR(test_case_name, test_name) static picotest::Registrar PICOTEST_JOIN(PICOTEST_IDENITY(test_case_name, test_name), _registrar)
#define PICOTEST_MAKE_TEST(test_case_name, test_name) picotest::Test(PICOTEST_STR(test_name), PICOTEST_TEST_CASE_INVOKER(test_case_name, test_name))

/////////////////////////////////////////////////////////////////
// test with auto-registration

#define TEST(test_case_name, test_name) \
PICOTEST_TEST_CASE_AUTO_REGISTER(test_case_name, test_name, ::testing::Test)


#define TEST_F(test_fixture, test_name) \
PICOTEST_TEST_CASE_AUTO_REGISTER(test_fixture, test_name, test_fixture)


#define PICOTEST_TEST_CASE_AUTO_REGISTER(test_case_name, test_name, base_t) \
struct PICOTEST_IDENITY(test_case_name, test_name) : public base_t {        \
	void test_method();                                                     \
};                                                                          \
	                                                                        \
void PICOTEST_TEST_CASE_INVOKER(test_case_name, test_name)() {              \
    PICOTEST_IDENITY(test_case_name, test_name) t;                          \
	t.execute();                                                            \
}                                                                           \
                                                                            \
PICOTEST_TEST_CASE_REGISTRAR(test_case_name, test_name)(                    \
	PICOTEST_STR(test_case_name),                                           \
	PICOTEST_MAKE_TEST(test_case_name, test_name));                         \
	                                                                        \
void PICOTEST_IDENITY(test_case_name, test_name)::test_method()


/////////////////////////////////////////////////////////////////
// EXPECT_XX

#define EXPECT_BOOL(expected, actual) picotest::detail::assertBool(expected, actual, foo::condStr(#expected, #actual, picotest::detail::EQ()), __FILE__, __LINE__)
#define EXPECT_BINARY(left, right, OP) picotest::detail::assertBinary(left, right, PICOTEST_JOIN(picotest::detail::, OP()), foo::condStr(#left, #right, picotest::detail::OP()), __FILE__, __LINE__)

#define EXPECT_TRUE(cond) EXPECT_BOOL(true, cond)
#define EXPECT_FALSE(cond) EXPECT_BOOL(false, cond)
#define EXPECT_EQ(expected, actual) EXPECT_BINARY(expected, actual, EQ)
#define EXPECT_LT(expected, actual) EXPECT_BINARY(expected, actual, LT)
#define EXPECT_GT(expected, actual) EXPECT_BINARY(expected, actual, GT)
#define EXPECT_LE(expected, actual) EXPECT_BINARY(expected, actual, LE)
#define EXPECT_GE(expected, actual) EXPECT_BINARY(expected, actual, GE)
#define EXPECT_STREQ(expected_str, actual_str) EXPECT_BINARY(expected_str, actual_str, STREQ)
#define EXPECT_STRNE(expected_str, actual_str) EXPECT_BINARY(expected_str, actual_str, STRNE)
#define EXPECT_STRCASEEQ(expected_str, actual_str) EXPECT_BINARY(expected_str, actual_str, STRCASEEQ)
#define EXPECT_STRCASENE(expected_str, actual_str) EXPECT_BINARY(expected_str, actual_str, STRCASENE)

/////////////////////////////////////////////////////////////////
// ASSERT_XX

#define ASSERT_BOOL(expected, actual) \
do {\
	if (!EXPECT_BOOL(expected, actual)){\
		return;\
	}\
} while(0)

#define ASSERT_BINARY(left, right, OP) \
do {\
	if (!EXPECT_BINARY(left, right, OP)){\
		return;\
	}\
} while(0)

#define ASSERT_TRUE(cond) ASSERT_BOOL(true, cond)
#define ASSERT_FALSE(cond) ASSERT_BOOL(false, cond)
#define ASSERT_EQ(expected, actual) ASSERT_BINARY(expected, actual, EQ)
#define ASSERT_LT(expected, actual) ASSERT_BINARY(expected, actual, LT)
#define ASSERT_GT(expected, actual) ASSERT_BINARY(expected, actual, GT)
#define ASSERT_LE(expected, actual) ASSERT_BINARY(expected, actual, LE)
#define ASSERT_GE(expected, actual) ASSERT_BINARY(expected, actual, GE)
#define ASSERT_STREQ(expected_str, actual_str) ASSERT_BINARY(expected_str, actual_str, STREQ)
#define ASSERT_STRNE(expected_str, actual_str) ASSERT_BINARY(expected_str, actual_str, STRNE)
#define ASSERT_STRCASEEQ(expected_str, actual_str) ASSERT_BINARY(expected_str, actual_str, STRCASEEQ)
#define ASSERT_STRCASENE(expected_str, actual_str) ASSERT_BINARY(expected_str, actual_str, STRCASENE)

/////////////////////////////////////////////////////////////////
// RUNNING ALL TESTS

#define RUN_ALL_TESTS() \
picotest::Registry::getInstance().testRun(); \
picotest::Registry::getInstance().report(std::cout); \
picotest::Registry::getInstance().fail();
