#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

bool isnumeric(const std::string &s)
{
	bool ok = 1;
	for (char c : s) {
		ok &= std::isdigit(c);
	}
	return ok;
}

map <int, vector <int> > graph;

string get_proc_name(std::string p_name)
{
	std::string res;
	ifstream command("/proc/" + p_name + "/" + "comm");
	getline(command, res);
	command.close();
	return res;
}

void build_tree(std::string p_name)
{
	ifstream ppid("/proc/" + p_name + "/" + "stat");
	std::string temp;
	int parent;
	ppid >> temp >> temp >> temp >> parent;
	if (graph.find(parent) == graph.end())
		graph[parent] = vector<int>();
	graph[parent].push_back(stoi(p_name));
	ppid.close();
}

int tree(const char* dir, int depth) {
	DIR* d = opendir(dir);
	struct dirent *entry;
	int i;
	char next_[512];
	if(!d) { 
		closedir(d);
		return 1;
	}
	while ((entry = readdir(d)) != 0) {
		next_[0] = 0;
		if(entry->d_name[0] == '.') 
			continue;
		if ((entry->d_type == DT_DIR) && (isnumeric(entry->d_name)))
		{
			std::cout << entry->d_name << "\n";
			build_tree(entry->d_name);
		}
	}
	closedir(d);
	return 0;
}

void print_tree(int v=1, int d=0)
{
	for (int i = 0; i < d; ++i)
		cout << "\t";
	cout << get_proc_name(std::to_string(v)) << endl;
	if (graph.find(v) == graph.end()) {
		return;
	}
	for (int to : graph[v]) {
		if (to == v || to == 0)
			continue;
		print_tree(to, d + 1);
	}
}

int main(int argc, char** argv){
	tree("/proc", 0);
	for (auto id: graph) {
		cout << id.first << ": ";
		for (int sub : id.second) {
			cout << sub << " ";
		}
		cout << endl;
	}
	cout << "__" << endl;
	print_tree();
	return 0;
}
