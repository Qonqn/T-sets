#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// 定义四叉树节点结构体
struct AsqtNode
{
    int space_code;
    int node_type; //0 represents split by y;1 represents split by x;2 represents leaf node
    double median;
    double median_1;
    double median_2;
    AsqtNode* child[4];
};

void Split(const std::string& src, const std::string& separator, std::vector<std::string>& dest) //字符串分割到数组
{
	//参数1：要分割的字符串；参数2：作为分隔符的字符；参数3：存放分割后的字符串的vector向量
	string str = src;
	string substring;
	string::size_type start = 0, index;
    dest.clear();
	index = str.find_first_of(separator, start);
	do{
		if(index != string::npos){
			substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = index + separator.size();
            index = str.find(separator, start);
			if (start == string::npos) break;
		}

	}while (index != string::npos);
	//the last part
    substring = str.substr(start);
    dest.push_back(substring);

}

AsqtNode* createNode(ifstream& file){
    string line;
    getline(file, line);
    //cout<<line<<endl;
    std::istringstream iss(line);
    line.pop_back();

    // 逐个读取分隔的字段
    std::vector<std::string> tokens;
    Split(line," ",tokens);
    AsqtNode* newnode=new AsqtNode;
    if(tokens.size()==2){
        newnode->node_type=2;       
    }
    else if(tokens.size()==5){
        newnode->node_type=stoi(tokens[1]);
        newnode->median = std::stod(tokens[2]);
        if(tokens[3]=="None"){
            tokens[3]="0";
        }
        newnode->median_1 = std::stod(tokens[3]);
        if(tokens[4]=="None"){
            tokens[4]="0";
        }
        newnode->median_2 = std::stod(tokens[4]); 
    }
    else{
        cout<<"tokens'size error"<<endl; 
    }
    return newnode;
}

// 读取文件并构建四叉树
AsqtNode* buildAsqtTree(ifstream& file)
{
    AsqtNode* root=createNode(file);
    if(root->node_type!=2){
        for(int i=0;i<4;i++){
            root->child[i]=buildAsqtTree(file);
        }
    }
    else{
        for(int i=0;i<4;i++){
            root->child[i]=NULL;
        }
    }
    return root;
}

void printAsqtTree(AsqtNode* root, int level = 0)
{
    if (root != nullptr)
    {
        std::cout << "Level " << level << " - Space Code: " << root->space_code
                  << ", Node Type: " << root->node_type
                  << ", Median: " << root->median
                  << ", Median1: " << root->median_1
                  << ", Median2: " << root->median_2 << std::endl;

        for (int i = 0; i < 4; ++i)
        {
            printAsqtTree(root->child[i], level + 1);
        }
    }
}

