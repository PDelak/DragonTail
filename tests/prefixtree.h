#pragma once

#include "../cogecs/prefixtree.h"

TEST(prefixtree, test1)
{
	PrefixTree tree;
	tree.insert("ab", 0);
	tree.insert("abc", 1);
	auto result = tree.search("d");
	EXPECT_FALSE(result);
	result = tree.search("ab");
	EXPECT_TRUE(result);
	EXPECT_EQ(result->getValue(), 0);
	result = tree.search("abc");
	EXPECT_EQ(result->getValue(), 1);
}
