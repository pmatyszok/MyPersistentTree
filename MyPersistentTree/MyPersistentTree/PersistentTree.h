#pragma once
#include <memory>
#include <iterator>
namespace Persistent
{
	template <typename T>
	class TreeIterator;

	template <typename T, typename Allocator=std::allocator<T>>
	class Tree
	{
		friend class TreeIterator<T>;
		struct Node;
	public:
		typedef T ValueType;
		typedef Allocator AllocatorType;
		Tree()
			: root(nullptr)
		{

		}

		/// temporary for testing ; this class should comply more or less to std::(multi)set interface
		void insert(T value)
		{
			pointer cand = root, newNode = std::make_shared<Node>(value), tmp = nullptr;;
			while (cand != nullptr)
			{
				tmp = cand;
				if (value < *(cand->value))
				{
					cand = cand->left;
				}
				else
				{
					cand = cand->right;
				}
			}
			newNode->parent = tmp;
			if (tmp == nullptr)
			{
				this->root = newNode;
			}
			else
			{
				if (*(newNode->value) < *(tmp->value))
					tmp->left = newNode;
				else
					tmp->right = newNode;
			}
		}

		TreeIterator<T> begin()
		{
			return TreeIterator<T>(root);
		}

	private:
		typedef typename std::shared_ptr<Node> pointer;
		
		
		struct Node
		{
			pointer left;
			pointer right;
			pointer parent;
			std::shared_ptr<ValueType> value;

			Node(ValueType value)
				: left(nullptr), right(nullptr), parent(nullptr), value(std::allocate_shared<Tree::ValueType>(Tree::AllocatorType(), value))
			{

			}

			pointer MinimumNode(pointer node)
			{
				while (node->left != nullptr)
				{
					node = node->left;
				}
				return node;
			}

			pointer Next()
			{
				if (this->right != nullptr)
					return MinimumNode(this->right);
				std::shared_ptr<Node> cand = this->parent, r = nullptr;
				while (cand != nullptr && (r = cand->right) != nullptr)
				{
					r = cand;
					cand = cand->parent;
				}
				return cand;

			}
		};

		std::shared_ptr<Node> root;

	public:

	};

	template<typename T>
	class TreeIterator : public std::iterator<std::forward_iterator_tag, T>
	{
	public:
		typedef typename std::shared_ptr<typename Tree<T>::Node> NodeType;
	private:
		NodeType current;
	public:
		TreeIterator(NodeType item) :
			current(item)
		{

		}
		
		bool operator != (const TreeIterator<T> & rhs)
		{
			return this->current != rhs.current;
		}

		T operator* () const
		{
			return *(current->value);
		}

		TreeIterator<T>& operator++()
		{
			current = current->Next();
			return *this;
		}
	};
}