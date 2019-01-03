#pragma once

#include "../cogecs/prefixtree.h"

TEST(prefixtree, test1)
{
	PrefixTree tree;
	tree.insert("abc");
	auto result = tree.search("d");
	EXPECT_FALSE(result);
	result = tree.search("ab");
	EXPECT_TRUE(result);
}
