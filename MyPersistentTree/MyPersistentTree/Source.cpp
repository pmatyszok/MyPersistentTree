#include "PersistentTree.h"
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cassert>


using namespace Persistent;
using namespace std;



struct verify;

class tester {
	friend verify;
	static int livecount;
	const tester* self;
	int d;
public:
	tester(int i) 	 
	{ 
		self = this;
		d = i;
		++livecount; 
	}
	
	tester(const tester& rhs)
	{
		self = this;
		d = rhs.d;
		++livecount; 
	}

	~tester() 
	{ 
		assert(self == this);--livecount; 
	}

	tester& operator=(const tester& b) 
	{
		assert((self == this) && b.self == &b);
		return *this;
	}
	bool operator <(const tester& rhs) 
	{ 
		return (this->d) < rhs.d; 
	}
	bool operator == (const tester& rhs) 
	{ 
		return (this->d) == rhs.d; 
	}
	bool operator !=(const tester& rhs) 
	{ 
		return !(*this == rhs); 
	}
	void cfunction() const 
	{ 
		assert(self == this);
	}
	void mfunction() 
	{ 
		assert(self == this); 
	}
};

int tester::livecount = 0;

struct verify {
	~verify() 
	{ 
		assert(tester::livecount == 0); 
	}
} verifier;
template <typename T>
void check( Tree<T>& t, const T& v)
{
	auto it = t.find(v);
	if (it == t.end())
	{
		cout << v << " not found" << endl;
	}
	else
	{
		cout << "found " << v << endl;
	}
}

int main()
{
	// basic testing
	Tree<int> t;
	cout << "Tree time on creation: " << t.GetTime() << '\t' << t.GetMaxTime() << endl;
	check(t, 1);
	t.insert(1);
	cout << "Tree time after first insertion: " << t.GetTime() << '\t' << t.GetMaxTime() << endl;
	check(t, 1);
	cout << "Changing time to " << 0 << endl;
	t.SetTime(0);
	check(t, 1);
	t.ResetTreeTime();
	t.insert(2);
	t.insert(3);
	t.insert(4);
	t.insert(5);
	t.insert(6);

	cout << "Tree time after 6 insertions: " << t.GetTime() << endl;
	cout << "Max time after 6 insertions: " << t.GetMaxTime() << endl;
	cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	cout << "Changing time to " << 5 << endl;
	t.SetTime(5);
	cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	cout << "Changing time to " << 4 << endl;
	t.SetTime(4);
	cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	cout << "Changing time to " << 3 << endl;
	t.SetTime(3);
	cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	cout << "Changing time to " << 2 << endl;
	t.SetTime(2);
	cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	cout << "Changing time to " << 1 << endl;
	t.SetTime(1);
	cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	cout << "Changing time to " << 0 << endl;
	t.SetTime(0);
	if (t.begin() != t.end())
		cout << "\"First\" value in tree at that time: " << (*t.begin()) << endl;
	
	auto it = t.find(5);
	if (it != t.end())
		cout << *it << endl;
	else
		cout << "5 not found" << endl;

	it = t.find(1);
	if (it != t.end())
		cout << *it << endl;
	else
		cout << "1 not found" << endl;

	cout << "Changing time to " << 5 << endl;
	t.SetTime(5);

	it = t.find(5);
	if (it != t.end())
		cout << *it << endl;
	else
		cout << "5 not found" << endl;

	cout << "Changing time to " << 4 << endl;

	it = t.find(5);
	if (it != t.end())
		cout << *it << endl;
	else
		cout << "5 not found" << endl;
	t.ResetTreeTime();
	it = t.find(5);
	if (it != t.end())
		it++;
	cout << *it << endl;
	it = t.begin();
	advance(it, 1);
	cout << *(it) << endl;
	t.remove(1);
	// verifier approach
	tester a(1);
	a = tester(2);



	Tree<tester> t2;
	t2.insert(tester(1));
	t2.insert(tester(2));
	t2.clear();
	system("pause");
}