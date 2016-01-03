#pragma once
#include <memory>
#include <iterator>
namespace Persistent
{
	template <typename T>
	class TreeIterator;

	template < typename T, typename A = std::allocator<T>, typename TIME_T = unsigned int >
	class Tree
	{
		struct Node;
		typedef typename std::shared_ptr<Node> pointer_;
		typedef typename std::weak_ptr<Node> weak_pointer;
	public:
		typedef A allocator_type;
		typedef typename A::value_type value_type;
		typedef typename A::reference reference;
		typedef typename A::const_reference const_reference;
		typedef typename A::difference_type difference_type;
		typedef typename A::size_type size_type;

		class iterator
		{
		public:
			typedef typename A::difference_type difference_type;
			typedef typename A::value_type value_type;
			typedef typename A::reference reference;
			typedef typename A::pointer pointer;
			typedef std::random_access_iterator_tag iterator_category;

			typedef typename std::shared_ptr<typename Tree<T>::Node> NodeType;
		private:
			NodeType current;
		public:
			iterator(NodeType item) :
				current(item)
			{

			}

			iterator(const iterator & rhs)
			{
				this->current = rhs.current;
			}

			~iterator()
			{
				current = nullptr;;
			}

			iterator& operator= (const iterator& rhs)
			{
				if (this->current == rhs.current)
					return *this;
				this->current = rhs.current;
				return *this;
			}

			bool operator == (const iterator& rhs) const
			{
				return this->current == rhs.current;
			}

			bool operator != (const iterator& rhs) const
			{
				return this->current != rhs.current;
			}

			iterator& operator += (size_type count)
			{
				while (count--)
					this->operator++();
				return *this;
			}

			reference operator* () const
			{
				return *(current->value);
			}

			pointer operator -> () const
			{
				return current->value;
			}

			iterator& operator++()
			{
				current = current->Next();
				return *this;
			}
			
			iterator& operator++(int)
			{
				current = current->Next();
				return *this;
			}
		};

		Tree()
			: root(nullptr), sentinel(nullptr), time(0)
		{

		}

		TIME_T GetTime() { return time; }

		/// temporary for testing ; this class should comply more or less to std::(multi)set interface
		void clear()
		{
			for (iterator it = this->begin(); it != this->end(); it++)
			{
				remove(it);
			}
		}


		void insert(T value)
		{
			pointer_ cand = root, newNode = std::make_shared<Node>(value), tmp = nullptr;;
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

		iterator find(const T& val)
		{
			pointer_ node = find_internal(val);
			
			if (node == nullptr)
				return end();
			return iterator(node);
		}

		void remove(const iterator& it)
		{
			this->remove(*it);
		}

		pointer_ remove(const T& val)
		{
			pointer_ to_remove = find_internal(val);
			if (to_remove == nullptr)
			{
				return nullptr;
			}
			
			pointer_ y, x;
			if (to_remove->left == nullptr || to_remove == nullptr)
			{
				y = to_remove;
			}
			else
			{
				y = to_remove->Next();
			}
			if (y->left != nullptr)
			{
				x = y->left;
			}
			else
			{
				x = y->right;
			}
			if (x != nullptr)
			{
				x->parent = y->parent.lock();
			}
			if (y->parent.lock() == nullptr)
			{
				root = x;
			}
			else
			{
				if (y == y->parent.lock()->left)
				{
					y->parent.lock()->left = x;
				}
				else
				{
					y->parent.lock()->right = x;
				}
			}
			if (y != to_remove)
			{
				to_remove->value = y->value;
				to_remove->parent = y->parent.lock();
				to_remove->left = y->left;
				to_remove->right = y->right;
			}
			return y;
		}

		iterator begin()
		{
			return iterator(root);
		}

		iterator end()
		{
			if (root == nullptr)
				return begin();
			return iterator(sentinel);
		}

	private:
		pointer_ root;
		pointer_ sentinel;
		TIME_T time;
		
		pointer_ find_internal(T val)
		{
			auto node = root;
			while ((node != nullptr) && (*(node->value) != val))
			{
				if (val < *(node->value))
				{
					node = node->left;
				}
				else
				{
					node = node->right;
				}
			}
			return node;
		}
		enum class Modification_t : unsigned int
		{
			None,
			Left,
			Right,
			Value
		};

		union Store_t
		{
			pointer_ node;
			std::shared_ptr<T> value;
			~Store_t()
			{

			}
		};

		struct ModInfo
		{
			const Modification_t kind;
			TIME_T time;
			Store_t store;
			
			explicit ModInfo(Modification_t which, const pointer_& node, const TIME_T t)
				: kind(which), time(t)
			{
				assert(kind == Modification_t::Left || kind == Modification_t::Right);
				store.node = node;
			}
			
			explicit ModInfo(Modification_t which, const std::shared_ptr<T>& value, const TIME_T t)
				: kind(which), time(t)
			{
				assert(kind == Modification_t::Value);
				store.value = value;
			}

			~ModInfo()
			{
			}
		};

		struct Node
		{
		private:
			pointer_ left;
			pointer_ right;
			weak_pointer parent;
			std::unique_ptr<ModInfo> modInfo;
			std::shared_ptr<T> value;

		public:
			Node(T value)
				: left(nullptr), right(nullptr), value(std::allocate_shared<T>(Tree::allocator_type(), value))
			{

			}

			//for tree recreation when going up
			Node(T value, const pointer_& left, const pointer_& right, const weak_pointer& parent)
				: this->left(left), this->right(right), this->parent(parent.lock())
			{

			}

			const pointer_ Left(const TIME_T when)
			{
				if (modInfo && modInfo->time <= when && modInfo->kind == Modification_t::Left)
				{
					return modInfo->store.node;
				}
				return left;
			}

			const pointer_ Right(const TIME_T when)
			{
				if (modInfo && modInfo->time <= when && modInfo->kind == Modification_t::Right)
				{
					return modInfo->store.node;
				}
				return right;
			}

			const std::shared_ptr<T> Value(const TIME_T when)
			{
				if (modInfo && modInfo->time <= when && modInfo->kind == Modification_t::Value)
				{
					return modInfo->store.value;
				}
				return value;
			}

			bool HasModification() const
			{
				return modInfo != nullptr;
			}
			
			void SetLeft(const pointer_ & newLeft, const TIME_T when)
			{
				assert(modInfo == nullptr);
				modInfo = std::make_unique<ModInfo>(Modification_t::Left, newLeft, when);
			}

			void SetRight(const pointer_ & newRight, const TIME_T when)
			{
				assert(modInfo == nullptr);
				modInfo = std::make_unique<ModInfo>(Modification_t::Right, newRight, when);
			}

			void SetValue(const std::shared_ptr<T>& newValue, const TIME_T when)
			{
				assert(modInfo == nullptr);
				modInfo = std::make_unique<ModInfo>(Modification_t::Value, newValue, when);
			}

			pointer_ MinimumNode(pointer_ node)
			{
				while (node->left != nullptr)
				{
					node = node->left;
				}
				return node;
			}

			pointer_ Next()
			{
				if (this->right != nullptr)
					return MinimumNode(this->right);
				pointer_ cand = this->parent.lock(), r = nullptr;
				while (cand != nullptr && (r = cand->right) != nullptr)
				{
					r = cand;
					cand = cand->parent.lock();
				}
				return cand;

			}
		};

		

	public:

	};
}