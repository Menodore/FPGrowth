#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include <set>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <memory>

using namespace std;

struct fpNode {
    string name;
    int val = 0;
    vector<shared_ptr<fpNode>> child{};
    shared_ptr<fpNode> parent = NULL;
    shared_ptr<fpNode> next = NULL;
    fpNode(string s) : name(s), next(NULL) {};
    fpNode(string s, int v, shared_ptr<fpNode> n) : name(s), val(v), parent(n), next(NULL) {};
};

struct link {
    int freq = 0;
    shared_ptr<fpNode> head = NULL;
    shared_ptr<fpNode> tail = NULL;
};

bool build_table(int&, map<string, link>&, vector<pair<string, int>>&, float&);
void constructFP(shared_ptr<fpNode>, map<string, link>&, int);
shared_ptr<fpNode> conditional_tree(map<string, link>&, vector<pair<vector<string>, int>>&, int);
void create_pattern(shared_ptr<fpNode>, map<string, link>&, int, string&, vector<string>, map<vector<string>, int>&);
map<vector<string>, int> mine(shared_ptr<fpNode>, map<string, link>&, vector<pair<string, int>>&, int, int);
void write_file(map<vector<string>, int>&,string, int);

bool comp_int_string(const pair<string, int> a, const pair<string, int> b) {
    if (a.second == b.second) {
        return a.first < b.first;
    }
    return a.second < b.second;
}

bool comp_greater_int(const pair<string, int> a, const pair<string, int> b) {
    if (a.second == b.second) {
        return a.first < b.first;
    }
    return a.second > b.second;
}

string round_4digit(float num) {
    stringstream ss;
    ss << fixed << setprecision(4) << num;
    return ss.str();
}

shared_ptr<fpNode> find_child(vector<shared_ptr<fpNode>>& child, string target) {
    for (auto it = child.begin(); it != child.end(); it++) {
        if ((*it)->name == target) {
            return (*it);
        }
    }
    return NULL;
}

int main() {
    float min_sup_pct = 0.5; // Modify the minimum support percentage as needed
    int total_transaction = 0;
    map<string, link> header_table;
    vector<pair<string, int>> frequency;

    // Reading input and building the header table directly
    if (!build_table(total_transaction, header_table, frequency, min_sup_pct)) {
        return 1;
    }
    int min_sup = (int)ceil(total_transaction * min_sup_pct);

    sort(frequency.begin(), frequency.end(), comp_int_string);

    shared_ptr<fpNode> root(new fpNode("root"));

    // Construct FP-Tree
    constructFP(root, header_table, min_sup);
    // Mine patterns from the FP-Tree
    map<vector<string>, int> ans = mine(root, header_table, frequency, min_sup, total_transaction);
    // Write the output to the file
    string out_file = "output_fp.txt";
    write_file(ans , out_file, total_transaction);
    return 0;
}

bool build_table(int& transaction, map<string, link>& header_table, vector<pair<string, int>>& freqency_vec, float& min_support_pct) {
    ifstream ifs("td.txt");  // Open input file "td.txt"
    if (!ifs) {
        cerr << "Failed to open input file 'td.txt'. Please ensure the file exists in the correct directory." << endl;
        return false;
    }

    string line;
    while (getline(ifs, line)) {
        stringstream ss(line);
        string item;
        transaction++;
        while (ss >> item) {  // Read space-separated items
            header_table[item].freq++;
        }
    }
    ifs.close();

    int min_sup = (int)ceil(transaction * min_support_pct);
    for (auto& x : header_table) {
        if (x.second.freq >= min_sup) {
            freqency_vec.push_back(make_pair(x.first, x.second.freq));
        }
    }
    return true;
}

void constructFP(shared_ptr<fpNode> root, map<string, link>& header_table, int min_sup) {
    ifstream ifs("td.txt");  // Open input file "td.txt"
    if (!ifs) {
        cerr << "Failed to open input file 'td.txt'. Please ensure the file exists in the correct directory." << endl;
        return;
    }

    string line;
    while (getline(ifs, line)) { // Read a transaction
        stringstream ss(line);
        string item_name;
        shared_ptr<fpNode> loc = root;
        vector<pair<string, int>> sorted_t;

        while (ss >> item_name) { // Add each transaction to the tree
            if (header_table[item_name].freq >= min_sup) {
                sorted_t.push_back(make_pair(item_name, header_table[item_name].freq));
            }
        }
        sort(sorted_t.begin(), sorted_t.end(), comp_greater_int); // Sort by descending frequency

        for (auto& item : sorted_t) {
            item_name = item.first;
            link* h_item = &header_table[item_name];
            if (h_item->freq >= min_sup) {
                shared_ptr<fpNode> node = find_child(loc->child, item_name);
                if (node != NULL) {
                    node->val++;
                    loc = node;
                }
                else {
                    shared_ptr<fpNode> tmp(new fpNode(item_name, 1, loc));
                    loc->child.push_back(tmp);
                    loc = tmp;
                }
                if (h_item->head == NULL) {
                    h_item->head = loc;
                }
                if (h_item->tail == NULL) {
                    h_item->tail = loc;
                }
                else if (loc->val == 1) {
                    h_item->tail->next = loc;
                    h_item->tail = loc;
                }
            }
        }
    }
    ifs.close();
}

shared_ptr<fpNode> conditional_tree(map<string, link>& element_table, vector<pair<vector<string>, int>>& pattern_base, int min_support) {
    if (pattern_base.empty()) {
        return NULL;
    }
    shared_ptr<fpNode> root(new fpNode("root"));
    for (int i = 0; i < pattern_base.size(); i++) {
        vector<string> a_pattern = pattern_base[i].first;
        int value_pattern = pattern_base[i].second;
        shared_ptr<fpNode> loc = root;
        for (int j = a_pattern.size() - 1; j >= 0; j--) {
            link* h_item = &element_table[a_pattern[j]];
            if (h_item->freq >= min_support) {
                shared_ptr<fpNode> node = find_child(loc->child, a_pattern[j]);
                if (node != NULL) {
                    node->val += value_pattern;
                    loc = node;
                }
                else {
                    shared_ptr<fpNode> tmp(new fpNode(a_pattern[j], value_pattern, loc));
                    loc->child.push_back(tmp);
                    loc = tmp;
                }
                if (h_item->head == NULL) {
                    h_item->head = loc;
                }
                if (h_item->tail == NULL) {
                    h_item->tail = loc;
                }
                else if ((loc->next == NULL) && (loc != h_item->tail)) {
                    h_item->tail->next = loc;
                    h_item->tail = loc;
                }
            }
        }
    }
    return root;
}

void create_pattern(shared_ptr<fpNode> root, map<string, link>& header_table, int min_support, string& current_suffix, vector<string> ready_suffix, map<vector<string>, int>& pattern_support) {
    ready_suffix.push_back(current_suffix);
    int suffix_val = header_table[current_suffix].freq;

    map<string, link> element_count_table;
    vector<pair<vector<string>, int>> pattern_base;
    shared_ptr<fpNode> leaf = header_table[current_suffix].head;
    for (; leaf != NULL; leaf = leaf->next) {
        int prev_val = leaf->val;
        vector<string> tmp_path;
        for (shared_ptr<fpNode> node = leaf->parent; node->name != "root"; node = node->parent) {
            element_count_table[node->name].freq += prev_val;
            tmp_path.push_back(node->name);
        }
        if (!tmp_path.empty()) {
            pattern_base.push_back(make_pair(tmp_path, prev_val));
        }
    }
    if (pattern_base.empty()) {
        pattern_support[ready_suffix] = suffix_val;
        return;
    }
    shared_ptr<fpNode> fp_tree = conditional_tree(element_count_table, pattern_base, min_support);
    if (fp_tree == NULL) {
        return;
    }
    vector<pair<string, int>> frequency;
    for (auto x : element_count_table) {
        if (x.second.freq >= min_support) {
            frequency.push_back(make_pair(x.first, x.second.freq));
        }
    }
    sort(frequency.begin(), frequency.end(), comp_int_string);
    for (auto item : frequency) {
        string base = item.first;
        create_pattern(fp_tree, element_count_table, min_support, base, ready_suffix, pattern_support);
    }
    pattern_support[ready_suffix] = suffix_val;
}

map<vector<string>, int> mine(shared_ptr<fpNode> root, map<string, link>& header_table, vector<pair<string, int>>& frequency, int min_support, int transactions) {
    map<vector<string>, int> pattern_support;
    for (auto item : frequency) {
        string base = item.first;
        vector<string> suffix;
        create_pattern(root, header_table, min_support, base, suffix, pattern_support);
    }
    return pattern_support;
}

/*void write_file(map<vector<string>, int>& ans, int total_transaction) {
    ofstream ofs("output_fp.txt", ios::out); // Open output file "output_fp.txt"
    if (!ofs.is_open()) {
        cout << "Failed to open output file 'output_fp.txt'.\n";
        return;
    }
    for (auto& pattern : ans) {
        float confidence = (float)pattern.second / total_transaction;
        ofs << "{ ";
        for (int i = pattern.first.s
        ize() - 1; i >= 0; i--) {
            ofs << pattern.first[i] << ' ';
        }
        ofs << "} : " << pattern.second << ", " << round_4digit(confidence) << endl;
    }
    ofs.close();
}*/
void write_file(map<vector<string>, int>& ans, string output_file, int total_transaction) {
    ofstream myfile;
    myfile.open(output_file);

    if (!myfile.is_open()) {
        cerr << "Unable to open file for writing." << endl;
        return;
    }

    for (auto& a : ans) {
        int n = a.first.size();
        for (int i = 0; i < n - 1; i++) {
            myfile << a.first[i] << " ";
        }
        myfile << a.first[n - 1] << " : " << a.second << endl;
    }

    myfile.close();
}
