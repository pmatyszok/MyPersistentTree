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
	auto it = t.begin();
	cout << *it << endl;
	++it;
	cout << *it << endl;
	it = t.begin();
	advance(it, 1);
	cout << *(it) << endl;
	system("pause");
}