#pragma once

#include "../cogecs/prefixtree.h"

TEST(prefixtree, test1)
{
	PrefixTree tree;
	tree.insert({ 0 }, -1);
	auto result = tree.search({ 0 });
	EXPECT_EQ(result->getValue(), -1);
	tree.insert({0,1}, 0);
	tree.insert({0,1,2}, 1);
	result = tree.search({ 3 });
	EXPECT_FALSE(result);
	result = tree.search({0,1});
	EXPECT_TRUE(result);
	EXPECT_EQ(result->getValue(), 0);
	result = tree.search({0,1,2});
	EXPECT_EQ(result->getValue(), 1);

}
