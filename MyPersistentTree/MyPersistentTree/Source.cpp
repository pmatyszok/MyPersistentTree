#include "PersistentTree.h"
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace Persistent;
using namespace std;


int main()
{
	Tree<int> t;
	t.insert(1);
	t.insert(2);
	t.insert(3);
	t.insert(4);
	t.insert(5);
	t.insert(6);
	auto it = t.find(5);
	cout << *it << endl;
	t.remove(5);
	it = t.find(5);
	if (it != t.end())
		cout << *it << endl;
	else
		cout << "5 not found" << endl;
	it = t.begin();
	cout << *it << endl;
	++it;
	cout << *it << endl;
	it = t.begin();
	advance(it, 1);
	cout << *(it) << endl;
	system("pause");
}