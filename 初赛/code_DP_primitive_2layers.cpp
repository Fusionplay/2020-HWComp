#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>
#include <string>
#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#define MAXNUM 0xFFFFFFFF
using namespace std;
typedef unsigned int uint;
//#define DEBUG_thDFS
//#define DEBUG
//#define DEBUG_FILE
#define DEBUG_TIME
//#define SUBMIT

#ifdef SUBMIT
const char readpath[] = "/data/test_data.txt";
const char writepath[] = "/projects/student/result.txt";
#else
const char readpath[] = "test_data.txt"; //线上提交前：这里的路径要修改
const char writepath[] = "my_result.txt";
#endif

#ifdef DEBUG_FILE
FILE *fd = fopen("debug.txt", "w");
#endif // DEBUG_FILE

struct Vertices
{
	vector<uint> to; //每个节点指向的节点的列表，出度
	vector<uint> from;  //每个节点被哪些节点指向的列表，入度。用于拓扑排序中递归清除出度为0的节点
	int to_num = 0; //节点自己的出度，应保证=to.size()
	int from_num = 0; //节点自己的入度，应保证=from.size()
};

struct Cycle
{
	uint arr[7]; //存环的各个节点
	int size; //存环的大小：3-7
	Cycle(uint a[], int s)
	{
		size = s;
		memcpy(arr, a, 4 * s);
		/*for (int i = 0; i < s; ++i)
			arr[i] = a[i];*/
#ifdef DEBUG_FILE
		fprintf(fd, "len=%d, st=%d, end=%d, ast=%d, aend=%d\n", s, arr[0], arr[s - 1], a[0], a[s - 1]);
#endif // DEBUG
	}

	//bool operator < (const Cycle& r)
	//{
	//	//同样大小：按字典序来排
	//	for (int i = 0; i < size; ++i)
	//	{
	//		if (arr[i] != r.arr[i])
	//			return arr[i] < r.arr[i];
	//	}
	//	return false;
	//}
};

//==================================================全局变量===========================================
//typedef unordered_map<uint, Vertices> maptype;
typedef vector<Vertices> maptype;
maptype g; //图
uint trace[8]; //环缓存
int endind = -1;    //环缓存的最后位置的下标，也即环缓存为1--endind，endind <= 7
vector<Cycle> res[8]; //3 4 5 6 7个节点的环
uint cur;
vector<uint> visit;
vector<unordered_map<uint, vector<uint>>> path_2; //2级路径索引
										//vector<unordered_map<uint, vector<uint>>> path_3; //3级路径索引
										//vector<uint> path3_reachable[TNUM];
vector<uint> path2_reachable;
vector<uint> in_path2;

vector<uint> nodes;
unordered_map<uint, uint> IDmap;
maptype::iterator iter;
uint num1, num2, num3;
uint nodenum;
vector<uint> nodetmp;
//==================================================全局变量结束========================================


void findCycle(uint v, uint layer)
{
	/*
	v: 当前遍历到的顶点编号
	layer: 当前是DFS的第几层。由main函数顶层调用时应设layer=1. layer最大为7。
	*/
	++endind;
	trace[endind] = v;
	visit[v] = 2;
	int ind;
	int len; //环的长度
	auto end = g[v].to.end();

	if (layer < 6)
	{
		/*auto it = lower_bound(g[v].to.begin(), end, cur);
		if (it != end && *it == cur && endind >= 2)
		{
			len = endind + 1;
			res[len - 3].emplace_back(trace, len);
			++it;
		}
		for (; it != g[v].to.end(); ++it)
		{
			if (g[*it].visit != 2)
				findCycle(*it, layer + 1);
		}*/
		

		for (uint vn : g[v].to)
		{
			if (vn < cur)
				continue;       //这2句很重要
			if (vn == cur && endind >= 2) //找到环了
			{
				len = endind + 1;
				res[len].emplace_back(trace, len);
			}
			else if (visit[vn] != 2)
				findCycle(vn, layer + 1);
		}
	}

	else //第6层
	{
		len = 6;
		/*auto it = lower_bound(g[v].to.begin(), end, cur);
		if (it != end && *it == cur)
		{
			res[len - 3].emplace_back(trace, len);
		}*/
		for (uint vn : g[v].to)
		{
			if (vn == cur) //6元环
			{
				res[len].emplace_back(trace, len);
				break;
			}
		}

		len = 7;
		if (path2_reachable[v] == 1)
		{
			++endind;
			for (uint k : path_2[cur][v])
			{
				if (visit[k] != 2)
				{
					trace[endind] = k;
					res[len].emplace_back(trace, len);
				}
			}
			--endind;
		}
	}


	//int visit = g[v].visit;
	//if (visit == 2) //可能遇到3-5元环
	//{
	//	//ind = get_trace(v);
	//	//if (ind != -1) //确实是环
	//	if (v == cur) //仅允许首尾都是cur的环:解决重复回路问题
	//	{
	//		//len = endind - ind;
	//		len = endind;
	//		if (len >= 3) //一定<=7，故无需额外判断
	//		{
	//			//res[len - 3].push_back(Cycle(trace + ind, len));
	//			res[len - 3].emplace_back(trace, len);
	//		}

	//	}
	//	//return; //这条路探索完毕，不存在仍需要继续DFS递归的情况，因为v=2
	//}

	//else if (layer < 6)
	//{
	//	g[v].visit = 2;
	//	++endind;
	//	for (uint vn : g[v].to)
	//	{
	//		if (vn < cur)
	//			continue;       //这2句很重要
	//		trace[endind] = vn; //这2句很重要
	//		findCycle(vn, layer + 1);
	//	}
	//	--endind;
	//	g[v].visit = 1;
	//}

	//else //layer = 6; 6,7元环在这里找到
	//{
	//	int i = v;
	//	for (uint k : g[i].to)
	//	{
	//		if (k >= cur) 
	//		{
	//			if (k == cur)//找到6元环
	//			{
	//				len = endind + 1;
	//				res[len - 3].emplace_back(trace, len);
	//			}

	//			else //寻找7元环
	//			{
	//				auto it = lower_bound(path_2[i][cur].begin(), path_2[i][cur].end(), k);
	//			}
	//		}
	//	}
	//}

	visit[v] = 1;
	--endind;
}


void build_path_index(uint nodenum)
{
	path_2.resize(nodenum);
	//for (int i = 0; i < nodenum; i++) 
	//for (uint j: nodes) //对每个节点j
	for (uint j = 0; j < nodenum; ++j)
	{
		//if (g[j].to_num == 0) //该点是否存在
		//	continue;
		for (uint k : g[j].from)
		{
			if (k < j || k == MAXNUM)
				continue;
			for (uint i : g[k].from)
			{
				if (i <= j || i == MAXNUM)
					continue;
				path_2[j][i].push_back(k);//i->k->j
			}
		}
	}

	//for (int i = 0; i < nodenum; i++)
	for (auto iter = path_2.begin(); iter != path_2.end(); ++iter)
	{
		for (auto &it : *iter)
		{
			sort(it.second.begin(), it.second.end()); //将i->k_i->j中的k_i按序排好
		}
	}
}


void read_file()
{
	//读文件
	FILE *fp = fopen(readpath, "r");

	//do 
	while (fscanf(fp, "%u,%u,%u", &num1, &num2, &num3) != EOF)
	{
		/*g[num1].to.push_back(num2);
		g[num2].from.push_back(num1);
		++g[num1].to_num;
		++g[num2].from_num;*/
		nodetmp.push_back(num1);
		nodetmp.push_back(num2);
	}
	fclose(fp);
#ifdef DEBUG
	printf("Read succeeded.\n");
	printf("Nodetmp total=%u\n", nodetmp.size());
#endif // DEBUG
}


void build_graph()
{
	nodes = nodetmp;
	sort(nodes.begin(), nodes.end());
	nodes.erase(unique(nodes.begin(), nodes.end()), nodes.end());
	nodenum = nodes.size();
	//建立ID映射
	num3 = 0;
	IDmap.reserve(nodenum);
	for (uint u : nodes)
		IDmap[u] = num3++;
	//真正建立图
	g.resize(nodenum);
	num3 = nodetmp.size();
	for (uint i = 0; i < num3; i += 2)
	{
		num1 = IDmap[nodetmp[i]];
		num2 = IDmap[nodetmp[i + 1]];
		g[num1].to.push_back(num2);
		g[num2].from.push_back(num1);
		++g[num1].to_num;
		++g[num2].from_num;
	}
#ifdef DEBUG
	printf("Build Graph succeeded.\n");
	printf("g Nodes total=%u, nodes total = %u\n", g.size(), nodes.size());
#endif // DEBUG
}


void topo()
{
	queue<uint> toque; //出度为0的点的队列
	queue<uint> fromque; //入度为0的点的队列
	for (uint i = 0; i < nodenum; ++i)
	{
		if (g[i].to_num == 0)
			toque.push(i);
		else
		{
			if (g[i].from_num == 0)
				fromque.push(i);
			sort(g[i].to.begin(), g[i].to.end()); //顺便给指向的节点排序
		}
	}
	//for (iter = g.begin(); iter != g.end(); ++iter) 
	//{
	//	sort(iter->to.begin(), iter->to.end());
	//	if (iter->to_num == 0) //出度为0
	//		toque.push(iter->first);
	//	else if (iter->second.from_num == 0) //入度为0
	//		fromque.push(iter->first);
	//}
	uint num;
	vector<uint> *vtmp;
	vector<uint>::iterator it;

	//递归删除入度为0的节点
	while (!fromque.empty())
	{
		num = fromque.front();
		fromque.pop();
		//在图中删除这一点：其所有子节点入度-1，子节点的父亲列表中删除该节点，若减到0则加入fromque
		for (uint t : g[num].to)
		{
			--g[t].from_num;
			if (g[t].from_num == 0)
				fromque.push(t);

			else
			{
				vtmp = &g[t].from;
				for (it = vtmp->begin(); it != vtmp->end(); ++it)
				{
					if (*it == num)
					{
						//it = vtmp->erase(it);
						*it = MAXNUM;
						break;
					}
				}
			}
		}

		//图中删除这一节点
		g[num].to_num = 0; //连clear都不用，到时候直接判断tonum是不是0就判断此点是否存在
						   /*iter = g.find(num);
						   iter = g.erase(iter); */
	}
#ifdef DEBUG
	printf("Cut 0-in-degree succeeded.\n");
#endif // DEBUG

	//递归删除出度为0的节点
	while (!toque.empty())
	{
		num = toque.front();
		toque.pop();
		//在图中删除这一点：其所有父节点出度-1，父节点的孩子列表中删除该节点，若减到0则加入toque
		for (uint t : g[num].from)
		{
			if (t == MAXNUM)
				continue;

			--g[t].to_num;
			if (g[t].to_num == 0)
				toque.push(t);
			else
			{
				vtmp = &g[t].to;
				it = lower_bound(vtmp->begin(), vtmp->end(), num);
				it = vtmp->erase(it);
				/*for (it = vtmp->begin(); it != vtmp->end(); ++it)
				{
				if (*it == num)
				{
				it = vtmp->erase(it);
				break;
				}
				}*/
			}
		}

		//图中删除这一节点
		//g[num].to.clear();
		/*iter = g.find(num);
		iter = g.erase(iter); */
	}
#ifdef DEBUG
	printf("Cut 0-out-degree succeeded.\n");
	printf("Node num Trimmed=%u\n", g.size());
#endif // DEBUG
}


void init()
{
	/*for (uint i = 0; i < TNUM; ++i)
	{
	path2_reachable[i].resize(nodenum, 0);
	path3_reachable[i].resize(nodenum, 0);

	in_path2[i].reserve(nodenum);
	in_path3[i].reserve(nodenum);

	visit.resize(nodenum, 0);
	endind[i] = -1;

	for (uint j = 0; j < 5; ++j)
	res[i][j].reserve(5000);

	}*/
	//path_2.resize(nodenum);
	path2_reachable.resize(nodenum, 0);
	in_path2.reserve(nodenum);
	visit.resize(nodenum, 0);
	for (uint i = 3; i <= 7; ++i)
		res[i].reserve(5000);
	//path_3.resize(nodenum);
}


void DFS()
{
#ifdef DEBUG_thDFS
	int cnt = 1;
	printf("DFS started.\n");
#endif // DEBUG

	for (uint i = 0; i < nodenum; ++i)
	{
		//if (iter->second.visit != 3)
		//cur = iter->first;
		if (g[i].to_num == 0) //该节点不存在
			continue;

		for (auto& jks : path_2[i])
		{
			path2_reachable[jks.first] = 1;
			in_path2.push_back(jks.first);
		}

		cur = i;
		findCycle(cur, 1);
		//g[cur].visit = 3;

		for (uint t : in_path2)
			path2_reachable[t] = 0;
		in_path2.clear();

#ifdef DEBUG_thDFS
		if (cnt % 200 == 0)
			printf("%d-th DFS succeeded.\n", cnt);
		++cnt;
#endif // DEBUG

	}

#ifdef DEBUG
	printf("DFS succeeded.\n");
#endif // DEBUG
}


void write_file()
{
	uint sum = 0;
	for (int i = 3; i < 8; ++i)
	{
		//sort(res[i].begin(), res[i].end());
		sum += res[i].size();
	}

	vector<string> idstr(nodenum);
	for (uint i = 0; i < nodenum; ++i)
		idstr[i] = to_string(nodes[i]);

	FILE *fw = fopen(writepath, "w");
	fprintf(fw, "%u\n", sum); //先写入环的总数
	char strtmp[128];
	int len;
	for (Cycle& c : res[3])
	{
		len = sprintf(strtmp, "%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str());
		fwrite(strtmp, len, 1, fw);
	}
	for (Cycle& c : res[4])
	{
		len = sprintf(strtmp, "%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str());
		fwrite(strtmp, len, 1, fw);
	}
	for (Cycle& c : res[5])
	{
		len = sprintf(strtmp, "%s,%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str(), idstr[c.arr[4]].c_str());
		fwrite(strtmp, len, 1, fw);
	}
	for (Cycle& c : res[6])
	{
		len = sprintf(strtmp, "%s,%s,%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str(), idstr[c.arr[4]].c_str(), idstr[c.arr[5]].c_str());
		fwrite(strtmp, len, 1, fw);
	}
	for (Cycle& c : res[7])
	{
		len = sprintf(strtmp, "%s,%s,%s,%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str(), idstr[c.arr[4]].c_str(), idstr[c.arr[5]].c_str(), idstr[c.arr[6]].c_str());
		fwrite(strtmp, len, 1, fw);
	}

	fclose(fw);
	//写入文件中
	//FILE *fw = fopen(writepath, "w");
	//clock_t write_st = clock();
	//fprintf(fw, "%u\n", sum); //先写入环的总数
	//for (Cycle& c : res[0])
	//	fprintf(fw, "%u,%u,%u\n", nodes[c.arr[0]], nodes[c.arr[1]], nodes[c.arr[2]]);
	//for (Cycle& c : res[1])
	//	fprintf(fw, "%u,%u,%u,%u\n", nodes[c.arr[0]], nodes[c.arr[1]], nodes[c.arr[2]], nodes[c.arr[3]]);
	//for (Cycle& c : res[2])
	//	fprintf(fw, "%u,%u,%u,%u,%u\n", nodes[c.arr[0]], nodes[c.arr[1]], nodes[c.arr[2]], nodes[c.arr[3]], nodes[c.arr[4]]);
	//for (Cycle& c : res[3])
	//	fprintf(fw, "%u,%u,%u,%u,%u,%u\n", nodes[c.arr[0]], nodes[c.arr[1]], nodes[c.arr[2]], nodes[c.arr[3]], nodes[c.arr[4]], nodes[c.arr[5]]);
	//for (Cycle& c : res[4])
	//	fprintf(fw, "%u,%u,%u,%u,%u,%u,%u\n", nodes[c.arr[0]], nodes[c.arr[1]], nodes[c.arr[2]], nodes[c.arr[3]], nodes[c.arr[4]], nodes[c.arr[5]], nodes[c.arr[6]]);
}


int main()
{
	//读文件
#ifdef DEBUG_TIME
	clock_t timestart = clock(), timeend;
	clock_t read_st = timestart, read_end;
#endif

	read_file();

#ifdef DEBUG_TIME
	read_end = clock();
#endif


	//建立图
#ifdef DEBUG_TIME
	clock_t buildst = clock(), buildend;
#endif

	build_graph();

#ifdef DEBUG_TIME
	buildend = clock();
#endif


	//拓扑排序来递归删除入度和出度为0的节点
	//首先得遍历一遍图，将当前所有入度和出度为0的入队列
#ifdef DEBUG_TIME
	clock_t topost = clock(), topoend;
#endif

	topo();

#ifdef DEBUG_TIME
	topoend = clock();
#endif


	//开始DFS
#ifdef DEBUG_TIME
	clock_t builddfsst = clock(), builddfsend;
#endif
	init();
	build_path_index(nodenum);

	DFS();
#ifdef DEBUG_TIME
	builddfsend = clock();
#endif
	

	//写文件
#ifdef DEBUG_TIME
	clock_t writest = clock(), writeend;
#endif
	write_file();
#ifdef DEBUG_TIME
	writeend = clock();
#endif


#ifdef DEBUG_TIME
	timeend = clock();
	double read, build, topo, builddfs, write, total;
	read = (double)(read_end - read_st) / CLOCKS_PER_SEC;
	build = (double)(buildend - buildst) / CLOCKS_PER_SEC;
	topo = (double)(topoend - topost) / CLOCKS_PER_SEC;
	builddfs = (double)(builddfsend - builddfsst) / CLOCKS_PER_SEC;
	write = (double)(writeend - writest) / CLOCKS_PER_SEC;
	total = (double)(timeend - timestart) / CLOCKS_PER_SEC;
	printf("Read time = %lf s, %lf%%\n", read, read / total);
	printf("Build time = %lf s, %lf%%\n", build, build / total);
	printf("Topo time = %lf s, %lf%%\n", topo, topo / total);
	printf("Build & DFS time = %lf s, %lf%%\n", builddfs, builddfs / total);
	printf("Write time = %lf s, %lf%%\n", write, write / total);
	printf("Total time: %lf s\n", total);
#endif // DEBUG_TIME

	return 0;
}