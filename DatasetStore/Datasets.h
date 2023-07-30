#pragma once
#include"Dataset.h"
#include<list>
#include<limits.h>
#include<set>

struct dataset_node
{
	int index;
	int len;
};

struct char_node {
	int ascii;
	list<dataset_node> res;
};


class Datasets {
	int total_len;
	vector<Dataset> datasets;
	map<char, char_node> search_map;
	map<string, string> file_table;
public:
    shared_ptr<ZRedisConnection> con;
	Datasets();
	void Print();
	void Add(string path,shared_ptr<ZRedisConnection> con);
	void Delete(string name);
	vector<Dataset> search_by_name(string key_word);
	vector<Dataset> search_by_size(int begin,int end);
	void read_file_table();
};

Datasets::Datasets() {
	total_len = 0;
	ZRedisConnectionPool::init("127.0.0.1",6379,"123456",3);
	con = ZRedisConnectionPool::Get();
}


void Datasets::Print() {
	for(int i=0;i<datasets.size();i++){
		datasets[i].print();
	}

}

void Datasets::read_file_table() {


}

vector<int> getNext(string str)
{
	if (str.size() == 1)
	{
		return { -1 };
	}
	vector<int>next(str.size());
	next[0] = -1;
	next[1] = 0;
	int i = 2;
	int cn = 0;
	while (i < next.size())
	{
		if (str[i - 1] == str[cn])
		{
			next[i++] = ++cn;
		}
		else if (cn > 0)
		{
			cn = next[cn];
		}
		else
		{
			next[i++] = 0;
		}
	}
	return next;
}


bool kmp(string str1, string str2) {
	if (str2.size() < 1 || str1.size() < str2.size())
	{
		return false;
	}
	vector<int>next = getNext(str2);  //���ú�����ȡnext����
	int p1 = 0;             //����ֱ�ָ��str1��str2��ָ��
	int p2 = 0;
	while (p1 < str1.size() && p2 < str2.size())
	{
		if (str1[p1] == str2[p2])   //����ַ���ͬ����ָ��ָ����һ���ַ�
		{
			p1++;
			p2++;
		}
		else if (p2 == 0)           //ʵ��ƥ�䲻��
		{
			p1++;
		}
		else
		{
			p2 = next[p2];
		}
	}
	if (p2 == str2.size()) return true;
	else return false;
}


void Datasets::Add(string path,shared_ptr<ZRedisConnection> con) {
	Dataset newdataset(path,con,total_len);
	datasets.push_back(newdataset);
	total_len++;
	set<char> char_set;
	for (int i = 0; i < newdataset.name.size(); i++) {
		if (char_set.find(newdataset.name[i]) != char_set.end())
			continue;
		else
			char_set.insert(newdataset.name[i]);
		if (search_map.count(newdataset.name[i]) == 0) {
			char_node new_char_node;
			new_char_node.ascii = (int)newdataset.name[i];
			dataset_node new_dataset_node;
			new_dataset_node.index = total_len - 1;
			new_dataset_node.len = newdataset.name.size();
			new_char_node.res.push_back(new_dataset_node);
			search_map[newdataset.name[i]] = new_char_node;
		}
		else {
			dataset_node new_dataset_node;
			new_dataset_node.index = total_len - 1;
			new_dataset_node.len = newdataset.name.size();
			search_map[newdataset.name[i]].res.push_back(new_dataset_node);
		}
	}
}

void Datasets::Delete(string name) {
	int index = 0;
	for (int i = 0; i < datasets.size(); i++) {
		if (datasets[i].name == name) {
			index = i;
			break;
		}
	}//��ȷ��λ�ã����Ҫ����ɾ��
	dataset_node target;
	target.index = index;
	target.len = name.size();
	for (int i = 0; i < name.size(); i++) {
		
		for (list<dataset_node>::iterator it = search_map[name[i]].res.begin(); it != search_map[name[i]].res.end(); it++)
		{
			if ((*it).index == target.index) {
				search_map[name[i]].res.erase(it);
				break;
			}
		}
	}
	string last_dataset_name = datasets[datasets.size() - 1].name;
	for (int i = 0; i < name.size(); i++) {

		for (list<dataset_node>::iterator it = search_map[name[i]].res.begin(); it != search_map[name[i]].res.end(); it++)
		{
			if ((*it).index == datasets.size() - 1) {
				(*it).index = index;
			}
		}

	}
	datasets[index] = datasets[datasets.size() - 1];
	datasets.pop_back();
}

vector<Dataset> Datasets::search_by_name(string key_word) {
	vector<Dataset> res;
	int min_len = INT_MAX;
	int min_index = 0;
	for (int i = 0; i < key_word.size(); i++) {
		if (search_map.count(key_word[i]) == 0) {
			return res;
		}
		else {
			if (search_map[key_word[i]].res.size() < min_len) {
				min_len = search_map[key_word[i]].res.size();
				min_index = i;
			}
		}
	}
	for (list<dataset_node>::iterator it = search_map[key_word[min_index]].res.begin(); it != search_map[key_word[min_index]].res.end(); it++)
	{
		int index = (*it).index;
		if (kmp(datasets[index].name, key_word)) {
			res.push_back(datasets[index]);
			cout << datasets[index].name << " ";
		}
	}
	cout << endl;
	return res;
}

vector<Dataset> Datasets::search_by_size(int begin, int end) {
	

}