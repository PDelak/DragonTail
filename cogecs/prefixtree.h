#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

struct PrefixNode
{
	PrefixNode() :key(-1), is_leaf(false), value(0) {}
	PrefixNode(int k) :key(k), is_leaf(false), value(0) {}
	PrefixNode(int k, int val) :key(k), is_leaf(false), value(val) {}

	bool exists(int k) const { return children.find(k) != children.end(); }

	PrefixNode* getChild(int k) 
	{
		auto n = children.find(k);
		if (n == children.end()) return nullptr;
		return n->second.get();
	}
	void setChild(int key, PrefixNode* n)
	{
		children.insert(std::make_pair(key, std::unique_ptr<PrefixNode>(n)));
	}

	int getKey() const { return key; }
	int getValue() const { return value; }

	std::unordered_map<int, std::unique_ptr<PrefixNode>>&  getChildren() { return children; }

	bool isLeaf() const { return is_leaf; }
	void setLeaf() { is_leaf = true; }

private:
	std::unordered_map<int, std::unique_ptr<PrefixNode>> children;
	int key;
	bool is_leaf;
	int value;

};

struct PrefixTree
{

	PrefixTree() :root(new PrefixNode) {}

	void insert(const std::vector<int>& key, int value) 
	{
		PrefixNode* current = root.get();

		for (auto k : key) {
			if (!current->exists(k)) 
			{
				auto tmpNode = new PrefixNode(k, value);
				current->setChild(k, tmpNode);
			}
			current = current->getChild(k);
		}
		current->setLeaf();
	}

	PrefixNode* search(const std::vector<int>& key)
	{
		auto current = root.get();
		for (auto character : key) 
		{
			if (!current->exists(character)) return nullptr;
			current = current->getChild(character);
		}
		return current;
	}

	std::string longestCommonPrefix()
	{
		std::string lcs;
		PrefixNode* current = root.get();

		while (current && current->getChildren().size() == 1 && !current->isLeaf()) 
		{
			char character = current->getChildren().begin()->second->getKey();
			lcs.push_back(character);

			current = current->getChildren().begin()->second.get();

		}
		return lcs;
	}

	void visit() {
		std::string word;
		visit(root.get(), word);
	}
private:

	void visit(PrefixNode* ptr, std::string& word)
	{
		word.push_back(ptr->getKey());
		if (ptr->isLeaf())
			std::cout << word.c_str() << std::endl;
		std::unordered_map<int, std::unique_ptr<PrefixNode>>&  children = ptr->getChildren();
		for (auto begin = children.begin(); begin != children.end(); ++begin) 
		{
			visit(begin->second.get(), word);
			word.pop_back();
		}
	}

	std::unique_ptr<PrefixNode> root;
};

