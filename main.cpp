#include <iostream>
#include <unordered_map>

class Trie {
 public:
  struct Node {
    uint64_t times = 0;
    std::unordered_map<char, Node*> children;
    Node* max_child = nullptr;
    char next_probable_letter = '0';
  };

 public:
  Trie();
  ~Trie();
  void AddText(std::string&& text);
  std::string Request(std::string&& prefix);
  std::string Append(std::string&& appendix);

 private:
  using WordsCount = std::unordered_map<std::string, uint64_t>;

  void Clear(Node*& root);

  static WordsCount ParseText(const std::string& text);
  void AddWord(const std::string& word, uint64_t times);

  Node* AddWordTo(Node* root, const std::string& word, uint64_t from, uint64_t
  times);

  Node* Find(Node* root, const std::string& word);
  std::string FindContinuation();

 private:
  Node* root_;
  Node* last_node_ = nullptr;
  std::string last_word;
};

Trie::Trie(): root_(new Node()) {}

void Trie::Clear(Trie::Node*& root) {
  if(root != nullptr) {
    for(auto&& [c, child]: root->children) {
      if(child != nullptr) {
        Clear(child);
      }
    }
    delete root;
  }
}

Trie::~Trie() {
  Clear(root_);
}

Trie::WordsCount Trie::ParseText(const std::string& text) {
  WordsCount words;
  uint64_t length = text.length();

  for(uint64_t it1 = 0; it1 < length; ) {
    uint64_t it2 = it1;
    for(; it2 < length && text[it2] != ' ' ; ++it2);
    ++words[text.substr(it1, it2 - it1)];
    for(; it2 < length && (text[it2] == ' ' || text[it2] == '\n') ; ++it2);
    it1 = it2;
  }
  return words;
}

Trie::Node* Trie::AddWordTo(Trie::Node* root, const std::string& word,
    uint64_t from, uint64_t times) {
  if(from < word.length()) {
    Node*& next = root->children[word[from]];
    if(next == nullptr) {
      next = new Node();
    }

    Node* end_node = AddWordTo(next, word, from + 1, times);
    if(root->max_child == nullptr || end_node->times > root->max_child->times) {
      root->max_child = end_node;
      root->next_probable_letter = word[from];
    }
    return end_node;
  }

  root->times += times;
  if(root->max_child == nullptr || root->times > root->max_child->times) {
    root->max_child = root;
    root->next_probable_letter = '\0';
  }
  return root;
}

void Trie::AddWord(const std::string& word, uint64_t times) {
  AddWordTo(root_, word, 0, times);
}

void Trie::AddText(std::string&& text) {
  auto parsed_text = ParseText(std::move(text));
  for(auto&& [word, times]: parsed_text) {
    AddWord(word, times);
    //std::cout << word << " : " << times << std::endl;
  }
}

Trie::Node* Trie::Find(Node* root, const std::string& word) {
  if(root == nullptr) {
    return nullptr;
  }
  Node* current = root;
  for(auto c: word) {
    Node* next = current->children[c];
    if(next == nullptr) {
      break;
    }
    current = next;
  }
  return current;
}

std::string Trie::FindContinuation() {
  if(last_node_ == nullptr) {
    return last_word;
  }
  Node* target = last_node_->max_child;
  if(target == nullptr) {
    return last_word;
  }
  std::string result = last_word;
  while(last_node_ != target) {
    result.push_back(last_node_->next_probable_letter);
    last_node_ = last_node_->children[last_node_->next_probable_letter];
  }

  return result;
}

std::string Trie::Request(std::string&& prefix) {
  last_node_ = Find(root_, prefix);
  last_word = prefix;

  return FindContinuation();
}

std::string Trie::Append(std::string&& appendix) {
  last_node_ = Find(last_node_, appendix);
  last_word += appendix;

  return FindContinuation();
}

enum Command {
  undefined = 0,
  add_text,
  request,
  append,
  end
};

std::unordered_map<std::string, Command> str_to_command = {
    {"ADD", Command::add_text},
    {"REQUEST", Command::request},
    {"APPEND", Command::append},
    {"END", end}
};

Command ParseCommand(const std::string& command) {
  return str_to_command[command];
}

std::string Input() {
  std::string input;
  std::cin >> input;
  return input;
}

Command CommandInput() {
  return ParseCommand(Input());
}

std::string TextInput() {
  std::string input;
  std::getline(std::cin, input);
  std::getline(std::cin, input, '\n');
  return input;
}

void AddText(Trie& dictionary) {
  std::string text = TextInput();
  dictionary.AddText(std::move(text));
}

void Request(Trie& dictionary) {
  std::string input = Input();
  std::cout << "Hint: " << dictionary.Request(std::move(input)) << std::endl;
}

void Append(Trie& dictionary) {
  std::string input = Input();
  std::cout << "Hint: " << dictionary.Append(std::move(input)) << std::endl;
}

void HandleCommand(Command command, Trie& dictionary) {
  switch (command) {
    case Command::add_text:
      AddText(dictionary);
      break;
    case Command::request:
      Request(dictionary);
      break;
    case Command::append:
      Append(dictionary);
      break;
    case Command::end:
      return;
    default:
      return;
  }
}

void Work() {
  Trie dictionary;
  Command command = Command::undefined;

  while(command != Command::end) {
    command = CommandInput();
    HandleCommand(command, dictionary);
  }
}

int main() {
  Work();

  return 0;
}
