#include "modules.hpp"

class BinaryTreeFixture
{
public:
    class Test
    {
    public:
        int number;
        std::string str;

        Test() : number(0), str()
        {
        }

        Test(const std::string &str) : number(0), str(str)
        {
        }

        Test(int number, const std::string &str) : number(number), str(str)
        {
        }

        ~Test()
        {
        }

        bool operator<(const Test &obj) const
        {
            return (this->str.compare(obj.str) < 0);
        }

        bool operator>(const Test &obj) const
        {
            return (this->str.compare(obj.str) > 0);
        }

        bool operator==(const Test &obj) const
        {
            return (this->str.compare(obj.str) == 0);
        }
    };

    std::vector<std::string> expectedResult = {
        "one",
        "three",
        "two",
        "zero"};

    BinaryTree<Test> test;

    BinaryTreeFixture() : test()
    {
        test.insert(Test(0, "zero"));
        test.insert(Test(1, "one"));
        test.insert(Test(2, "two"));
        test.insert(Test(3, "three"));
        CHECK(test.size() == 4);
    }

    ~BinaryTreeFixture()
    {
    }

    void findTest(const std::string &str, bool isFound, int expected = 0) const
    {
        const Node<Test> *f = test.find(Test(str));
        if (isFound)
        {
            CHECK(f != nullptr);
            if (f)
            {
                CHECK(f->data.number == expected);
            }
        }
        else
        {
            CHECK(f == nullptr);
        }
    }
};

TEST_CASE_FIXTURE(BinaryTreeFixture, "inOrder")
{
    int idx = 0;
    test.inOrder(
        [&](const Test &t)
        {
            CAPTURE(t.str);
            CAPTURE(expectedResult.at(idx));
            CHECK(t.str.compare(expectedResult.at(idx)) == 0);
            idx++;
            return true; });
}

TEST_CASE_FIXTURE(BinaryTreeFixture, "inOrder")
{
    int idx = 0;
    test.inOrder(
        [&](const Test &t)
        {
            CAPTURE(t.str);
            CAPTURE(expectedResult.at(idx));
            CHECK(t.str.compare(expectedResult.at(idx)) == 0);
            idx++;
            return true; });
}

TEST_CASE_FIXTURE(BinaryTreeFixture, "contains")
{
    CHECK(test.contains(Test("zero")) == true);
    CHECK(test.contains(Test("one")) == true);
    CHECK(test.contains(Test("two")) == true);
    CHECK(test.contains(Test("three")) == true);
}

TEST_CASE_FIXTURE(BinaryTreeFixture, "find")
{
    findTest("zero", true, 0);
    findTest("one", true, 1);
    findTest("two", true, 2);
    findTest("three", true, 3);
}

TEST_CASE_FIXTURE(BinaryTreeFixture, "remove")
{
    test.remove(Test("one"));
    CHECK(test.size() == 3);
    CHECK(test.contains(Test("one")) == false);
    findTest("zero", true, 0);
    findTest("one", false, 0);
    findTest("two", true, 2);
    findTest("three", true, 3);
}

TEST_CASE_FIXTURE(BinaryTreeFixture, "clear")
{
    test.clear();
    CHECK(test.size() == 0);
    test.inOrder(
        [](const Test &t)
        {
            CHECK(false);
            return true; });
}