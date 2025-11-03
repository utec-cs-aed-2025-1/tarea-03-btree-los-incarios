#ifndef BTree_H
#define BTree_H
#include <iostream>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n; // total de elementos en el arbol 

  // Helper functions
  bool searchHelper(Node<TK>* node, TK key);
  void insertNonFull(Node<TK>* node, TK key);
  void splitChild(Node<TK>* parent, int index);
  void removeHelper(Node<TK>* node, TK key);
  TK getPredecessor(Node<TK>* node, int idx);
  TK getSuccessor(Node<TK>* node, int idx);
  void fill(Node<TK>* node, int idx);
  void borrowFromPrev(Node<TK>* node, int idx);
  void borrowFromNext(Node<TK>* node, int idx);
  void merge(Node<TK>* node, int idx);
  int heightHelper(Node<TK>* node);
  void toStringHelper(Node<TK>* node, string& result, const string& sep);
  void rangeSearchHelper(Node<TK>* node, TK begin, TK end, vector<TK>& result);
  void clearHelper(Node<TK>* node);
  bool checkPropertiesHelper(Node<TK>* node, int& level, int currentLevel);

 public:
  BTree(int _M) : root(nullptr), M(_M), n(0) {}

  bool search(TK key);//indica si se encuentra o no un elemento
  void insert(TK key);//inserta un elemento
  void remove(TK key);//elimina un elemento
  int height();//altura del arbol. Considerar altura 0 para arbol vacio
  string toString(const string& sep);  // recorrido inorder
  vector<TK> rangeSearch(TK begin, TK end);

  TK minKey();  // minimo valor de la llave en el arbol
  TK maxKey();  // maximo valor de la llave en el arbol
  void clear(); // eliminar todos lo elementos del arbol
  int size(); // retorna el total de elementos insertados  
  
  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree* build_from_ordered_vector(vector<TK> elements, int M);
  // Verifique las propiedades de un árbol B
  bool check_properties();

  ~BTree();     // liberar memoria
};

// Search implementation
template <typename TK>
bool BTree<TK>::search(TK key) {
  return searchHelper(root, key);
}

template <typename TK>
bool BTree<TK>::searchHelper(Node<TK>* node, TK key) {
  if (node == nullptr) return false;
  
  int i = 0;
  while (i < node->count && key > node->keys[i]) {
    i++;
  }
  
  if (i < node->count && key == node->keys[i]) {
    return true;
  }
  
  if (node->leaf) {
    return false;
  }
  
  return searchHelper(node->children[i], key);
}

// Insert implementation
template <typename TK>
void BTree<TK>::insert(TK key) {
  if (root == nullptr) {
    root = new Node<TK>(M);
    root->keys[0] = key;
    root->count = 1;
    n++;
    return;
  }
  
  if (root->count == M - 1) {
    Node<TK>* newRoot = new Node<TK>(M);
    newRoot->leaf = false;
    newRoot->children[0] = root;
    splitChild(newRoot, 0);
    root = newRoot;
  }
  
  insertNonFull(root, key);
  n++;
}

template <typename TK>
void BTree<TK>::insertNonFull(Node<TK>* node, TK key) {
  int i = node->count - 1;
  
  if (node->leaf) {
    while (i >= 0 && key < node->keys[i]) {
      node->keys[i + 1] = node->keys[i];
      i--;
    }
    node->keys[i + 1] = key;
    node->count++;
  } else {
    while (i >= 0 && key < node->keys[i]) {
      i--;
    }
    i++;
    
    if (node->children[i]->count == M - 1) {
      splitChild(node, i);
      if (key > node->keys[i]) {
        i++;
      }
    }
    insertNonFull(node->children[i], key);
  }
}

template <typename TK>
void BTree<TK>::splitChild(Node<TK>* parent, int index) {
  Node<TK>* fullChild = parent->children[index];
  Node<TK>* newChild = new Node<TK>(M);
  newChild->leaf = fullChild->leaf;
  
  int mid = (M - 1) / 2;
  newChild->count = M - 1 - mid - 1;
  
  for (int j = 0; j < newChild->count; j++) {
    newChild->keys[j] = fullChild->keys[j + mid + 1];
  }
  
  if (!fullChild->leaf) {
    for (int j = 0; j <= newChild->count; j++) {
      newChild->children[j] = fullChild->children[j + mid + 1];
    }
  }
  
  fullChild->count = mid;
  
  for (int j = parent->count; j > index; j--) {
    parent->children[j + 1] = parent->children[j];
  }
  parent->children[index + 1] = newChild;
  
  for (int j = parent->count - 1; j >= index; j--) {
    parent->keys[j + 1] = parent->keys[j];
  }
  parent->keys[index] = fullChild->keys[mid];
  parent->count++;
}

// Remove implementation
template <typename TK>
void BTree<TK>::remove(TK key) {
  if (root == nullptr) return;
  
  removeHelper(root, key);
  
  if (root->count == 0) {
    Node<TK>* oldRoot = root;
    if (root->leaf) {
      root = nullptr;
    } else {
      root = root->children[0];
    }
    delete oldRoot;
  }
  n--;
}

template <typename TK>
void BTree<TK>::removeHelper(Node<TK>* node, TK key) {
  int idx = 0;
  while (idx < node->count && node->keys[idx] < key) {
    idx++;
  }
  
  if (idx < node->count && node->keys[idx] == key) {
    if (node->leaf) {
      for (int i = idx + 1; i < node->count; i++) {
        node->keys[i - 1] = node->keys[i];
      }
      node->count--;
    } else {
      int minKeys = (M - 1) / 2;
      if (node->children[idx]->count > minKeys) {
        TK pred = getPredecessor(node, idx);
        node->keys[idx] = pred;
        removeHelper(node->children[idx], pred);
      } else if (node->children[idx + 1]->count > minKeys) {
        TK succ = getSuccessor(node, idx);
        node->keys[idx] = succ;
        removeHelper(node->children[idx + 1], succ);
      } else {
        merge(node, idx);
        removeHelper(node->children[idx], key);
      }
    }
  } else {
    if (node->leaf) {
      return;
    }
    
    bool isInSubtree = (idx == node->count);
    int minKeys = (M - 1) / 2;
    
    if (node->children[idx]->count <= minKeys) {
      fill(node, idx);
    }
    
    if (isInSubtree && idx > node->count) {
      removeHelper(node->children[idx - 1], key);
    } else {
      removeHelper(node->children[idx], key);
    }
  }
}

template <typename TK>
TK BTree<TK>::getPredecessor(Node<TK>* node, int idx) {
  Node<TK>* curr = node->children[idx];
  while (!curr->leaf) {
    curr = curr->children[curr->count];
  }
  return curr->keys[curr->count - 1];
}

template <typename TK>
TK BTree<TK>::getSuccessor(Node<TK>* node, int idx) {
  Node<TK>* curr = node->children[idx + 1];
  while (!curr->leaf) {
    curr = curr->children[0];
  }
  return curr->keys[0];
}

template <typename TK>
void BTree<TK>::fill(Node<TK>* node, int idx) {
  int minKeys = (M - 1) / 2;
  
  if (idx != 0 && node->children[idx - 1]->count > minKeys) {
    borrowFromPrev(node, idx);
  } else if (idx != node->count && node->children[idx + 1]->count > minKeys) {
    borrowFromNext(node, idx);
  } else {
    if (idx != node->count) {
      merge(node, idx);
    } else {
      merge(node, idx - 1);
    }
  }
}

template <typename TK>
void BTree<TK>::borrowFromPrev(Node<TK>* node, int idx) {
  Node<TK>* child = node->children[idx];
  Node<TK>* sibling = node->children[idx - 1];
  
  for (int i = child->count - 1; i >= 0; i--) {
    child->keys[i + 1] = child->keys[i];
  }
  
  if (!child->leaf) {
    for (int i = child->count; i >= 0; i--) {
      child->children[i + 1] = child->children[i];
    }
  }
  
  child->keys[0] = node->keys[idx - 1];
  
  if (!child->leaf) {
    child->children[0] = sibling->children[sibling->count];
  }
  
  node->keys[idx - 1] = sibling->keys[sibling->count - 1];
  
  child->count++;
  sibling->count--;
}

template <typename TK>
void BTree<TK>::borrowFromNext(Node<TK>* node, int idx) {
  Node<TK>* child = node->children[idx];
  Node<TK>* sibling = node->children[idx + 1];
  
  child->keys[child->count] = node->keys[idx];
  
  if (!child->leaf) {
    child->children[child->count + 1] = sibling->children[0];
  }
  
  node->keys[idx] = sibling->keys[0];
  
  for (int i = 1; i < sibling->count; i++) {
    sibling->keys[i - 1] = sibling->keys[i];
  }
  
  if (!sibling->leaf) {
    for (int i = 1; i <= sibling->count; i++) {
      sibling->children[i - 1] = sibling->children[i];
    }
  }
  
  child->count++;
  sibling->count--;
}

template <typename TK>
void BTree<TK>::merge(Node<TK>* node, int idx) {
  Node<TK>* child = node->children[idx];
  Node<TK>* sibling = node->children[idx + 1];
  
  child->keys[child->count] = node->keys[idx];
  
  for (int i = 0; i < sibling->count; i++) {
    child->keys[child->count + 1 + i] = sibling->keys[i];
  }
  
  if (!child->leaf) {
    for (int i = 0; i <= sibling->count; i++) {
      child->children[child->count + 1 + i] = sibling->children[i];
    }
  }
  
  for (int i = idx + 1; i < node->count; i++) {
    node->keys[i - 1] = node->keys[i];
  }
  
  for (int i = idx + 2; i <= node->count; i++) {
    node->children[i - 1] = node->children[i];
  }
  
  child->count += sibling->count + 1;
  node->count--;
  
  delete sibling;
}

// Height implementation
template <typename TK>
int BTree<TK>::height() {
  return heightHelper(root);
}

template <typename TK>
int BTree<TK>::heightHelper(Node<TK>* node) {
  if (node == nullptr) return 0;
  if (node->leaf) return 1;
  return 1 + heightHelper(node->children[0]);
}

// ToString implementation (inorder traversal)
template <typename TK>
string BTree<TK>::toString(const string& sep) {
  string result = "";
  toStringHelper(root, result, sep);
  if (!result.empty() && result.size() >= sep.size()) {
    result = result.substr(0, result.size() - sep.size());
  }
  return result;
}

template <typename TK>
void BTree<TK>::toStringHelper(Node<TK>* node, string& result, const string& sep) {
  if (node == nullptr) return;
  
  int i;
  for (i = 0; i < node->count; i++) {
    if (!node->leaf) {
      toStringHelper(node->children[i], result, sep);
    }
    result += to_string(node->keys[i]) + sep;
  }
  
  if (!node->leaf) {
    toStringHelper(node->children[i], result, sep);
  }
}

// Range search implementation
template <typename TK>
vector<TK> BTree<TK>::rangeSearch(TK begin, TK end) {
  vector<TK> result;
  rangeSearchHelper(root, begin, end, result);
  return result;
}

template <typename TK>
void BTree<TK>::rangeSearchHelper(Node<TK>* node, TK begin, TK end, vector<TK>& result) {
  if (node == nullptr) return;
  
  int i = 0;
  while (i < node->count && node->keys[i] < begin) {
    i++;
  }
  
  while (i < node->count) {
    if (!node->leaf) {
      rangeSearchHelper(node->children[i], begin, end, result);
    }
    
    if (node->keys[i] >= begin && node->keys[i] <= end) {
      result.push_back(node->keys[i]);
    }
    
    if (node->keys[i] > end) {
      return;
    }
    
    i++;
  }
  
  if (!node->leaf) {
    rangeSearchHelper(node->children[i], begin, end, result);
  }
}

// MinKey implementation
template <typename TK>
TK BTree<TK>::minKey() {
  if (root == nullptr) {
    throw runtime_error("Tree is empty");
  }
  Node<TK>* curr = root;
  while (!curr->leaf) {
    curr = curr->children[0];
  }
  return curr->keys[0];
}

// MaxKey implementation
template <typename TK>
TK BTree<TK>::maxKey() {
  if (root == nullptr) {
    throw runtime_error("Tree is empty");
  }
  Node<TK>* curr = root;
  while (!curr->leaf) {
    curr = curr->children[curr->count];
  }
  return curr->keys[curr->count - 1];
}

// Clear implementation
template <typename TK>
void BTree<TK>::clear() {
  clearHelper(root);
  root = nullptr;
  n = 0;
}

template <typename TK>
void BTree<TK>::clearHelper(Node<TK>* node) {
  if (node == nullptr) return;
  
  if (!node->leaf) {
    for (int i = 0; i <= node->count; i++) {
      clearHelper(node->children[i]);
    }
  }
  
  node->killSelf();
  delete node;
}

// Size implementation
template <typename TK>
int BTree<TK>::size() {
  return n;
}

// Build from ordered vector
template <typename TK>
BTree<TK>* BTree<TK>::build_from_ordered_vector(vector<TK> elements, int M) {
  if (elements.empty()) return nullptr;
  
  BTree<TK>* tree = new BTree<TK>(M);
  
  for (const auto& elem : elements) {
    tree->insert(elem);
  }
  
  return tree;
}

// Check properties
template <typename TK>
bool BTree<TK>::check_properties() {
  if (root == nullptr) return true;
  
  int level = -1;
  return checkPropertiesHelper(root, level, 0);
}

template <typename TK>
bool BTree<TK>::checkPropertiesHelper(Node<TK>* node, int& level, int currentLevel) {
  if (node == nullptr) return true;
  
  // Check if leaf level is consistent
  if (node->leaf) {
    if (level == -1) {
      level = currentLevel;
    } else if (level != currentLevel) {
      return false;
    }
  }
  
  // Check number of keys
  int minKeys = (M - 1) / 2;
  if (node != root && node->count < minKeys) {
    return false;
  }
  
  if (node->count > M - 1) {
    return false;
  }
  
  // Check if keys are sorted
  for (int i = 0; i < node->count - 1; i++) {
    if (node->keys[i] >= node->keys[i + 1]) {
      return false;
    }
  }
  
  // Recursively check children
  if (!node->leaf) {
    for (int i = 0; i <= node->count; i++) {
      if (!checkPropertiesHelper(node->children[i], level, currentLevel + 1)) {
        return false;
      }
    }
  }
  
  return true;
}

// Destructor
template <typename TK>
BTree<TK>::~BTree() {
  clear();
}

#endif
