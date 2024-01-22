#ifndef UTILS_HPP
#define UTILS_HPP

template <class T>
struct Node
{
	T data;
	Node *next;

	Node(T& data) : data(data), next(0) {  }
	static void Append(T& data, Node *&list);
	static void Remove(Node<T> *node, Node<T> *&list);
	static int Len(const Node<T> *list);
	static void Free(Node<T> *&list);
};

bool str_to_int(char *str, int& num);
char **split_line(char *line, int& argc);

template <class T>
void Node<T>::Append(T& data, Node<T> *&list)
{
	Node<T> *node = new Node<T>(data);
	if(!list) {
		list = node;
	}
	else {
		Node<T> *last = list;
		for(; last->next; last = last->next) 
		{  }

		last->next = node;
	}
}

template <class T>
void Node<T>::Remove(Node<T> *node, Node<T> *&list)
{
	if(list == node)
		list = node->next;
	else {
		Node<T> *tmp = list;
		while(node != tmp->next)
			tmp = tmp->next; 
		tmp->next = node->next;
	}
	
	delete node;
}

template <class T>
int Node<T>::Len(const Node<T> *list)
{
	return list ? Len(list->next) + 1 : 0;
}

template <class T>
void Node<T>::Free(Node<T> *&list)
{
	if(list)
	{
		Free(list->next);
		delete list;
	}
	list = 0;
}

#endif
