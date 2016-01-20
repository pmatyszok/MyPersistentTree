#include "PersistentTree.h"
#include <iostream>
#include <iterator>
#include <algorithm>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <fstream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <functional>
using namespace Persistent;
using namespace std;
using namespace std::chrono;



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

template<typename T>
void inorder(Tree<T>& t)
{
	for (auto it = t.begin(); it != t.end(); it++)
	{
		cout << *it << '\t';
	}
	cout << endl;
}


int main()
{
/*	Tree<int> k;
	k.insert(2);
	k.insert(1);
	k.insert(3);

	auto z = k.Clone();
	z->insert(4);
	cout << "Z: " << z->GetTime() << endl;
	check(*z, 4);
	check(*z, 3);

	cout << "K: " << k.GetTime() << endl;
	check(k, 4);
	check(k, 3);
	
	return 0;*/

	vector<string> words;

	fstream file("polish.dic");
	if (!file.is_open())
	{
		cout << "file not opened" << endl;
		system("pause");
		return -1;
	}

	std::copy(istream_iterator<string>(file),
		istream_iterator<string>(),
		std::back_inserter(words));

	cout << "Read all lines, shuffling!" << endl;
	srand((unsigned int)time(NULL));
	std::random_shuffle(words.begin(), words.end());
	cout << "Shuffled" << endl;
	cout << "Insert to set: " << endl;
	{
		set<string> s;
		auto start = high_resolution_clock::now();

		for (auto i = 0u; i < words.size(); i++)
		{
			s.insert(words[i]);
		}

		auto end = high_resolution_clock::now();
		auto time_span = duration_cast<duration<double>>(end - start);
	
		cout << "Took " << time_span.count() << " seconds." << endl;
	}
	

	

	cout << "Insert to persistent tree: " << endl;
	{
		Tree<string> t;
		auto start = high_resolution_clock::now();

		for (auto& w : words)
		{
			t.insert(w);
		}

		auto end = high_resolution_clock::now();
		auto time_span = duration_cast<duration<double>>(end - start);
		
		cout << "Took " << time_span.count() << " seconds." << endl;
	}
	

	system("pause");
}