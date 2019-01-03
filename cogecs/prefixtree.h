#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

struct PrefixNode
{
	PrefixNode() :character('$'), is_leaf(false), value(0) {}
	PrefixNode(char c) :character(c), is_leaf(false), value(0) {}
	PrefixNode(char c, size_t val) :character(c), is_leaf(false), value(val) {}

	bool exists(char c) const { return children.find(c) != children.end(); }

	PrefixNode* getChild(char c) {
		auto n = children.find(c);
		if (n == children.end()) return nullptr;
		return n->second.get();
	}
	void setChild(char c, PrefixNode* n)
	{
		children.insert(std::make_pair(c, std::unique_ptr<PrefixNode>(n)));
	}

	char getCharacter() const { return character; }
	size_t getValue() const { return value; }

	std::unordered_map<char, std::unique_ptr<PrefixNode>>&  getChildren() { return children; }

	bool isLeaf() const { return is_leaf; }
	void setLeaf() { is_leaf = true; }

private:
	std::unordered_map<char, std::unique_ptr<PrefixNode>> children;
	char character;
	bool is_leaf;
	size_t value;

};

struct PrefixTree
{

	PrefixTree() :root(new PrefixNode) {}

	void insert(const std::string& key, size_t value) {
		PrefixNode* current = root.get();

		for (auto character : key) {

			if (!current->exists(character)) {
				auto tmpNode = new PrefixNode(character, value);
				current->setChild(character, tmpNode);
			}
			current = current->getChild(character);
		}
		current->setLeaf();
	}

	PrefixNode* search(const std::string& key)
	{
		auto current = root.get();
		for (auto character : key) {
			if (!current->exists(character)) return nullptr;
			current = current->getChild(character);
		}
		return current;
	}

	std::string longestCommonPrefix()
	{
		std::string lcs;
		PrefixNode* current = root.get();

		while (current && current->getChildren().size() == 1 && !current->isLeaf()) {
			char character = current->getChildren().begin()->second->getCharacter();
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
		word.push_back(ptr->getCharacter());
		if (ptr->isLeaf())
			std::cout << word.c_str() << std::endl;
		std::unordered_map<char, std::unique_ptr<PrefixNode>>&  children = ptr->getChildren();
		for (auto begin = children.begin(); begin != children.end(); ++begin) {
			visit(begin->second.get(), word);
			word.pop_back();
		}
	}

	std::unique_ptr<PrefixNode> root;
};

