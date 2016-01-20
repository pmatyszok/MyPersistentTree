#pragma once
#include <memory>
#include <iterator>
#include <map>
#include <cassert>
#include <stack>
namespace Persistent
{
	template <typename T>
	class TreeIterator;

	template < typename T, typename A = std::allocator<T>, typename TIME_T = unsigned int >
	class Tree
	{
		struct Node;
		typedef typename std::shared_ptr<Node> pointer_;
		
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
			Tree<T, A, TIME_T>& tree;
			std::stack<NodeType> my_stack;
			TIME_T time;
		public:
			iterator(NodeType item, Tree<T, A, TIME_T>& tr, TIME_T t) :
				current(item), tree(tr), time(t)
			{
				auto cur = current;
				while (cur != nullptr)
				{
					my_stack.push(cur);
					cur = cur->Left(time);
				}
				current = next();
			}

			iterator(const iterator & rhs)
				: current(rhs.current), tree(rhs.tree), time(rhs.time), my_stack(rhs.my_stack)
			{

			}

			~iterator()
			{
				current = nullptr;
			}

			iterator& operator= (const iterator& rhs)
			{
				if (*this == rhs)
					return *this;
				this->current = rhs.current;
				this->time = rhs.time;
				this->tree = rhs.tree;
				this->my_stack = rhs.my_stack;
				return *this;
			}

			bool operator == (const iterator& rhs) const
			{
				return this->current == rhs.current && &(this->tree) == &(rhs.tree) && this->time == rhs.time && my_stack == rhs.my_stack;
			}

			bool operator != (const iterator& rhs) const
			{
				return !((*this) == rhs);
			}

			iterator& operator += (size_type count)
			{
				while (count--)
					this->operator++();
				return *this;
			}

			reference operator* () const
			{
				return *(current->Value(time));
			}

			pointer operator -> () const
			{
				return current->Value(time);
			}

			iterator& operator++()
			{
				current = next();
				return *this;
			}

			iterator& operator++(int)
			{
				current = next();
				return *this;
			}

		private:
			NodeType next()
			{
				if (my_stack.empty())
				{
					return nullptr;
				}
				auto top = my_stack.top();
				my_stack.pop();
				if (nullptr != top->Right(time))
				{
					auto cur = top->Right(time);
					while (nullptr != cur)
					{
						my_stack.push(cur);
						cur = cur->Left(time);
					}
				}
				return top;
			}
		};

		Tree()
			: root(nullptr), sentinel(nullptr), time(0), max_time(0)
		{

		}

		TIME_T GetMaxTime() { return max_time; }
		TIME_T GetTime() const { return time; }

		void ResetTreeTime()
		{
			time = max_time;
			this->root = this->roots.rbegin()->second;
		}

		void SetTime(TIME_T t)
		{
			if (t > max_time)
				throw std::runtime_error("Cannot set tree time from future!");

			time = t;

			if (time == 0)
			{
				this->root = nullptr;
				return;
			}

			auto candRoot = greatest_less_or_eq(this->roots, time);

			//get latest root if more suitable not found
			if (candRoot == this->roots.end())
			{
				this->root = this->roots.rbegin()->second;
			}
			else
			{
				this->root = candRoot->second;
			}
		}


		std::shared_ptr<Tree<T, A, TIME_T>> Clone()
		{
			auto tree = std::make_shared<Tree>();
			for (const auto r : roots)
			{
				tree->roots[r.first] = r.second->clone(GetTime());
			}
			tree->max_time = GetTime();
			tree->SetTime(GetTime());
			return tree;
		}

		void clear()
		{
			auto begin = this->begin();
			do
			{
				this->remove_impl(*begin);
				begin = this->begin();
			} while
				(begin != this->end());
			max_time++;
			time = max_time;
		}


		void insert(T value)
		{
			if (GetTime() < max_time)
				throw std::runtime_error("You cannot modify tree that has time set to past. Use ResetTreeTime method.");

			max_time++;
			time = max_time;

			pointer_ cand = root, tmp = nullptr;
			while (cand != nullptr)
			{
				tmp = cand;
				if (value < *(cand->Value(GetTime())))
				{
					cand = cand->Left(GetTime());
				}
				else
				{
					cand = cand->Right(GetTime());
				}
			}
			auto newNode = std::make_shared<Node>(value); //creates leaf
			if (tmp == nullptr)
			{
				this->root = newNode;
				this->roots[GetTime()] = newNode;
			}
			else
			{
				if (value < *(tmp->Value(GetTime())))
				{
					if (!tmp->HasModification())
					{
						// no modifications yet - insert new node
						tmp->SetLeft(newNode, GetTime());
					}
					else
					{
						TraverseUpAndCopyModifiedNodes(newNode, tmp, GetTime(), true);
					}
				}
				else
				{
					if (!tmp->HasModification())
						tmp->SetRight(newNode, GetTime());
					else
						TraverseUpAndCopyModifiedNodes(newNode, tmp, GetTime(), false);
				}
			}

		}

		void TraverseUpAndCopyModifiedNodes(pointer_ child, pointer_ parent, TIME_T t, bool isLeftChild)
		{
			pointer_ copy; //copied node with newNode pluged in correct place
			pointer_ tmp;
			do
			{
				if (isLeftChild)
					copy = std::make_shared<Node>(*(parent->Value(t)), child, parent->Right(t));
				else
					copy = std::make_shared<Node>(*(parent->Value(t)), parent->Left(t), child);
				tmp = get_parent(parent);

				if (parent == this->root)
				{
					this->roots[t] = copy;
					this->root = copy;
					return;
				}

				if (tmp->Left(t) == parent)
					isLeftChild = true;
				else
					isLeftChild = false;

				parent = tmp;
				child = copy;

			} while (parent != nullptr && parent->HasModification());

			//no more copies needed, modify top-most not-modified non-root node
			if (isLeftChild)
				parent->SetLeft(child, t);
			else
				parent->SetRight(child, t);
		}



		iterator find(const T& val)
		{
			pointer_ node = find_internal(val);

			if (node == nullptr)
				return end();
			return iterator(node, *this, GetTime());
		}

		void remove(const iterator& it)
		{
			this->remove(*it);
		}



		pointer_ remove(const T& val)
		{
			if (GetTime() < max_time)
				throw std::runtime_error("You cannot modify tree that has time set to past. Use ResetTreeTime method.");
			auto y = remove_impl(val);
			max_time++;
			time = max_time;
			return y;
		}

		iterator begin()
		{
			iterator beg(root, *this, GetTime());
			return beg;
		}

		iterator end()
		{
			if (root == nullptr)
				return begin();
			iterator e(sentinel, *this, GetTime());
			return e;
		}

	private:
		std::map<TIME_T, pointer_> roots;
		pointer_ root;
		pointer_ sentinel;
		TIME_T time;
		TIME_T max_time;

		template<typename Map> typename Map::iterator
			greatest_less_or_eq(Map & m, typename Map::key_type const& k) 
		{
			typename Map::iterator it = m.lower_bound(k);
			if (it->first == k)
				return it;
			if (it != m.begin()) 
			{
				return --it;
			}
			return m.end();
		}


		pointer_ remove_impl(const T& val)
		{
			pointer_ to_remove = find_internal(val);
			if (to_remove == nullptr)
			{
				return nullptr;
			}

			pointer_ y, x;
			if (to_remove->Right(GetTime()) == nullptr || to_remove->Left(GetTime()) == nullptr)
			{//none or one child
				y = to_remove;
			}
			else
			{//both childs, harder case
				y = Next(to_remove, GetTime());
			}


			if (y->Left(GetTime()) != nullptr)
			{
				x = y->Left(GetTime());
			}
			else
			{
				x = y->Right(GetTime());
			}

			if (get_parent(y) == nullptr)
			{
				root = x;
				this->roots[GetTime()] = x;
			}
			else
			{
				auto yparent = get_parent(y);
				bool hasMod = yparent->HasModification();
				if (y == yparent->Left(GetTime()))
				{
					if (!hasMod)
						yparent->SetLeft(x, GetTime());
					else
						TraverseUpAndCopyModifiedNodes(x, yparent, GetTime(), true);
				}
				else
				{
					if (!hasMod)
						yparent->SetRight(x, GetTime());
					else
						TraverseUpAndCopyModifiedNodes(x, yparent, GetTime(), false);
				}
			}
			if (y != to_remove)
			{
				//copy Value!
				if (to_remove->HasModification())
				{
					auto child = std::make_shared<Node>(*(y->Value(GetTime())), to_remove->Left(GetTime()), to_remove->Right(GetTime()));
					auto parent = get_parent(to_remove);
					if (parent != nullptr)
					{
						bool isLeft = to_remove == parent->Left(GetTime());
						TraverseUpAndCopyModifiedNodes(child, get_parent(to_remove), GetTime(), isLeft);
					}
					else
					{
						//root is being replaced
						this->roots[GetTime()] = child;
						this->root = child;
					}
				}
				else
				{
					to_remove->SetValue(y->Value(GetTime()), GetTime());
				}
			}
			return y;
		}

		pointer_ get_parent(const pointer_& n) const
		{
			auto node = root;
			pointer_ parent = nullptr;
			auto val = (*n->Value(GetTime()));
			while ((node != nullptr) && (*(node->Value(GetTime())) != val))
			{
				parent = node;
				if (val < (*(node->Value(GetTime()))))
					node = node->Left(GetTime());
				else
					node = node->Right(GetTime());
			}
			return parent;
		}

		pointer_ find_internal(T val) const
		{
			auto node = root;
			while ((node != nullptr) && (*(node->Value(GetTime())) != val))
			{
				if (val < *(node->Value(GetTime())))
				{
					node = node->Left(GetTime());
				}
				else
				{
					node = node->Right(GetTime());
				}
			}
			return node;
		}

		pointer_ Next(pointer_ node, const TIME_T when) const
		{
			if (node->Right(when) != nullptr)
				return Node::MinimumNode(node->Right(when), when);
			pointer_ cand = get_parent(node);
			while ((cand != nullptr) && (cand->Left(when) != node))
			{
				node = cand;
				cand = get_parent(cand);
			}
			return cand;

		}

		enum class Modification_t : unsigned int
		{
			None,
			Left,
			Right,
			Value
		};

		struct ModInfo
		{
			const Modification_t kind;
			TIME_T time;
			pointer_ node;
			std::shared_ptr<T> value;

			explicit ModInfo(Modification_t which, const pointer_& node, const TIME_T t)
				: kind(which), time(t)
			{
				assert(kind == Modification_t::Left || kind == Modification_t::Right);
				this->node = node;
			}

			explicit ModInfo(Modification_t which, const std::shared_ptr<T>& value, const TIME_T t)
				: kind(which), time(t)
			{
				assert(kind == Modification_t::Value);
				this->value = value;
			}

			bool operator ==(const ModInfo& rhs)
			{
				bool trival = time == rhs.time && kind == rhs.kind;
				if (!trival)
					return false;
				switch (kind)
				{
				case Modification_t::Left:
				case Modification_t::Right:
					return this->node == this->node;
				case Modification_t::Value:
					return this->value == this->value;
				default:
					assert(false);
				}
				return false;
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
			std::unique_ptr<ModInfo> modInfo;
			std::shared_ptr<T> value;

		public:
			Node(T value)
				: left(nullptr), right(nullptr), value(std::allocate_shared<T>(Tree::allocator_type(), value))
			{

			}

			//for tree recreation when going up
			Node(T value, const pointer_& left, const pointer_& right)
				: left(left), right(right), value(std::allocate_shared<T>(Tree::allocator_type(), value))
			{

			}

			std::shared_ptr<Node> clone(TIME_T when)
			{
				std::shared_ptr<Node> copy;
				if (left != nullptr && right != nullptr )
				{
					copy = std::make_shared<Node>(*(this->value), left->clone(when), right->clone(when)); // original value
				}
				else
				{
					copy = std::make_shared<Node>(*(this->value)); // original value
					
					if (left != nullptr)
					{
						copy->left = left->clone(when);
					}
					else if (right != nullptr)
					{
						copy->right = right->clone(when);
					}
				}
				if (modInfo)
				{
					switch (modInfo->kind)
					{
					case Modification_t::Value:
						copy->SetValue(this->Value(modInfo->time), modInfo->time); break;
					case Modification_t::Left:
						copy->SetLeft(this->Left(modInfo->time), modInfo->time); break;
					case Modification_t::Right:
						copy->SetRight(this->Right(modInfo->time), modInfo->time); break;
					}
				}
				return copy;
			}

			bool operator == (const Node& rhs) const
			{
				return left == rhs.left && right == rhs.right && value == rhs.value
					&& *modInfo == *rhs.modInfo; //modInfo is unique, perform value comparison
			}

			const pointer_ Left(const TIME_T when) const
			{
				if (modInfo && modInfo->time <= when && modInfo->kind == Modification_t::Left)
				{
					return modInfo->node;
				}
				return left;
			}

			const pointer_ Right(const TIME_T when) const
			{
				if (modInfo && modInfo->time <= when && modInfo->kind == Modification_t::Right)
				{
					return modInfo->node;
				}
				return right;
			}

			const std::shared_ptr<T> Value(const TIME_T when) const
			{
				if (modInfo && modInfo->time <= when && modInfo->kind == Modification_t::Value)
				{
					return modInfo->value;
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

			static pointer_ MinimumNode(pointer_ node, const TIME_T when)
			{
				while (node->Left(when) != nullptr)
				{
					node = node->Left(when);
				}
				return node;
			}
		};
	};
}