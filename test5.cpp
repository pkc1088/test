#include <iostream>
using namespace std;
int k;
template<typename T>
struct Node
{
	Node* left;
	Node* right;
	T value;
};
template <typename T>
class BinarySearchTree
{
public:
	BinarySearchTree() :root(nullptr) {};
	~BinarySearchTree() {};

	void AddNode(T _value);
	bool SearchValue(T _value);
	void RemoveNode(T _value);
	void DepthPrint(int dep);
	void Display();


private:
	Node<T>* root;
	Node<T>* tail;

	void Inorder(Node<T>* current)
	{
		if (current != nullptr)
		{	
			Inorder(current->left);
			cout << current->value << " ";
			Inorder(current->right);
		}
	}

	Node<T>* SearchMinNode(Node<T>* node)
	{
		if (node == NULL)
			return NULL;

		while (node->left != NULL)
		{
			node = node->left;
		}
		return node;
	}

	Node<T>* RemoveSeqence(Node<T>* node,T _vaule);
	void DepthPrint2(Node<T>* node, int depth);
};

template <typename T>
void BinarySearchTree<T>::DepthPrint2(Node<T>* node, int depth) {
	if (depth == k) {	
		cout << node->value << " ";
		return;
	} else  {
		DepthPrint2(node->right, depth + 1);
		DepthPrint2(node->left, depth + 1);
		return;
	}
}

template <typename T>
Node<T>* BinarySearchTree<T>::RemoveSeqence(Node<T>* node, T _value)
{
	if (node == nullptr) return node;
	
	else if (node->value.compare(_value) > 0)
		node->left = RemoveSeqence(node->left, _value);
	else if (node->value.compare(_value) < 0)
		node->right = RemoveSeqence(node->right, _value);
	else
	{
		Node<T>* ptr = node;
		//자식이없을떄
		if (node->right==nullptr && node->left==nullptr)
		{
			delete node;
			node = nullptr;
		}
		//자식이 하나일떄
		else if (node->right == nullptr)
		{
			node = node->left;
			delete ptr;
		}
		else if (node->left == nullptr)
		{
			node = node->right;
			delete ptr;
		}
		//자식이 두개일떄 :: 왼쪽 노드중 가장큰값 찾아 부모노드로 바꾸기
		else
		{
			ptr = SearchMinNode(node->right);
			node->value = ptr->value;
			node->right = RemoveSeqence(node->right, ptr->value);
		}
	}
	return node;
}

template <typename T>
void BinarySearchTree<T>::DepthPrint(int dep)
{
	Node<T>* ptr = root;
	DepthPrint2(ptr, dep);
}

template <typename T>
void BinarySearchTree<T>::RemoveNode(T _value)
{
	Node<T>* ptr = root;
	RemoveSeqence(ptr, _value);

}

template<typename T>
void BinarySearchTree<T>::Display()
{
	Inorder(root);
}

template <typename T>
bool BinarySearchTree<T>::SearchValue(T _value)
{
	Node<T>* ptr = root;
	Node<T>* tmpRoot = nullptr;

	while (ptr!=nullptr)
	{
		if (ptr->value == _value)
		{
			cout << _value << " 값을 찾았습니다." << endl;
			return true;
		}		
		else if (ptr->value > _value)
			ptr = ptr->left;
		else
			ptr = ptr->right;
	}
	cout << _value << " 값을 찾지 못했습니다." << endl;
	return false;
}

template <typename T>
void BinarySearchTree<T>::AddNode(T _value)
{
	Node<T>* node = new Node<T>();
	Node<T>* tmpRoot = nullptr;

	node->value = _value;

	if (root == nullptr)
		root = node;
	else
	{
		Node<T>* ptr = root;

		while (ptr != nullptr)
		{
			tmpRoot = ptr;
			if (node->value < ptr->value)
			{
				ptr = ptr->left;
			}
			else
			{
				ptr = ptr->right;
			}
		}
		//넣을 위치에 대입
		if (node->value < tmpRoot->value )
			tmpRoot->left = node;
		else
			tmpRoot->right = node;
	}
}
// https://xtar.tistory.com/40
int main()
{
	BinarySearchTree<string>* BST = new BinarySearchTree<string>();
	/*
	BST->AddNode("phone");
	BST->AddNode("banana");
	BST->RemoveNode("cola");
	BST->AddNode("chip");
	BST->Display(); cout << endl;
	BST->AddNode("pizza");
	BST->AddNode("soccer");
	BST->RemoveNode("phone");
	BST->AddNode("machine");
	BST->RemoveNode("pizza");
	*/
	BST->AddNode("30");
	BST->AddNode("20");
	BST->AddNode("25");
	BST->AddNode("40");
	BST->AddNode("50");
	BST->Display(); cout << endl;
	BST->DepthPrint(3);
	BST->RemoveNode("30");
	BST->Display(); cout << endl;

	
	return 0;
}
