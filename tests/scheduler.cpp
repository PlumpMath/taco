#include <basis/bitset.h>
#include <basis/unit_test.h>
#include <taco/taco.h>
#include <iostream>

#define TEST_TIMEOUT_MS 2000

void test_initialize_shutdown();
void test_schedule();
void test_switch();

BASIS_TEST_LIST_BEGIN()
	BASIS_DECLARE_TEST(test_initialize_shutdown)
	BASIS_DECLARE_TEST(test_schedule)
	BASIS_DECLARE_TEST(test_switch)
BASIS_TEST_LIST_END()

void test_initialize_shutdown()
{
	taco::Initialize();
	taco::Shutdown();
}

void test_schedule()
{
	taco::Initialize();

	int a = 0;

	auto start = basis::GetTimestamp();
	bool timeout = false;

	taco::Schedule([&]() -> void {
		a = 100;
	});

	while (a == 0)
	{
		if (basis::GetTimeElapsedMS(start) > TEST_TIMEOUT_MS)
		{
			timeout = true;
			break;
		}
	}

	taco::Shutdown();

	BASIS_TEST_VERIFY_MSG(!timeout, "Timed out after at least %d ms", TEST_TIMEOUT_MS);
	BASIS_TEST_VERIFY_MSG(timeout || a == 100, "Variable \"a\" set to an unexpected value - %d", a);
}

void test_switch()
{
	taco::Initialize(); 

	basis::bitset did_run(taco::GetThreadCount() + 1);

	// If the switching doesn't work then this task will hog the thread it gets grabbed by
	// So if we schedule more of these tasks than there are threads we'll never see all of them entered
	auto fn = [&](unsigned index) -> void {
		did_run.set_bit(index, 1);
		for (;;)
		{
			taco::Switch();
		}
	};

	auto start = basis::GetTimestamp();
	bool timeout = false;

	for (unsigned i=0; i<(taco::GetThreadCount() + 1); i++)
	{
		taco::Schedule(std::bind(fn, i));
	}

	while (!did_run.test_all_one())
	{
		if (basis::GetTimeElapsedMS(start) > TEST_TIMEOUT_MS)
		{
			timeout = true;
			break;
		}
	}

	taco::Shutdown();

	BASIS_TEST_VERIFY_MSG(!timeout, "timeout after at leat %d ms.  Not all tasks were entered.", TEST_TIMEOUT_MS);
}

int main()
{
	BASIS_RUN_TESTS();
	return 0;
}
