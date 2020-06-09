#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>
#include <string>
#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>
#include <queue>
#include <thread>
#include <mutex>
using namespace std;
typedef unsigned int uint;
#define MAXNUM 0xFFFFFFFF
#define TNUM 1           //线程数量

#define DEBUG_thDFS
#define DEBUG
#define DEBUG_TIME
//#define DEBUG_FILE
//#define DEBUG_PR
//#define SUBMIT

#ifdef SUBMIT
const char readpath[] = "/data/test_data.txt";
const char writepath[] = "/projects/student/result.txt";
#else
const char readpath[] = "test_data_big.txt"; //线上提交前：这里的路径要修改
const char writepath[] = "my_result_big.txt";
#endif

#ifdef DEBUG_FILE
FILE *fd = fopen("debug.txt", "w");
#endif // DEBUG

struct Vertices
{
	vector<uint> to; //每个节点指向的节点的列表，出度
	vector<uint> from;  //每个节点被哪些节点指向的列表，入度。用于拓扑排序中递归清除出度为0的节点
	int to_num = 0; //节点自己的出度，应保证=to.size()
	int from_num = 0; //节点自己的入度，应保证=from.size()
	//int visit = 0;      //遍历用的变量，0代表完全未见过，1代表见过但未遍历完其所有7层后代
	//2代表正处于某个DFS中, 3代表该节点完全7层后代都被看过
	//int reachable = 0;
};

struct Cycle
{
	int size; //存环的大小：3-7
	uint arr[7]; //存环的各个节点
	Cycle(uint a[], int s)
	{
		size = s;
		memcpy(arr, a, 4 * s);
#ifdef DEBUG_FILE
		fprintf(fd, "len=%d, st=%d, end=%d, ast=%d, aend=%d\n", s, arr[0], arr[s - 1], a[0], a[s - 1]);
#endif // DEBUG
	}

	bool operator < (const Cycle& r)
	{
		//同样大小：按字典序来排
		for (int i = 0; i < size; ++i)
		{
			if (arr[i] != r.arr[i])
				return arr[i] < r.arr[i];
		}
		return false;
	}
};


//==================================================全局变量===========================================
typedef vector<Vertices> maptype;
maptype g; //图
uint trace[8]; //环缓存
int endind = -1;    //环缓存的最后位置的下标，也即环缓存为1--endind，endind <= 7
vector<Cycle> res[8]; //3 4 5 6 7个节点的环
uint cur;
vector<uint> visit;
vector<map<uint, vector<uint>>> path_2; //2级路径索引
//vector<unordered_map<uint, vector<uint>>> path_3; //3级路径索引
//vector<uint> path3_reachable[TNUM];
vector<uint> path2_reachable;
vector<uint> in_path2;
//vector<uint> in_path3[TNUM];
//vector<uint> visit[TNUM];
//vector<Cycle> res[TNUM][5]; //3 4 5 6 7个节点的环
//uint trace[TNUM][8];
//int endind[TNUM];
//uint cur[TNUM];

vector<uint> nodes;
unordered_map<uint, uint> IDmap;
maptype::iterator iter;
uint num1, num2, num3;
uint nodenum;
vector<uint> nodetmp;
//==================================================全局变量结束========================================

//============================================方法=====================================================
void build_path_index(uint nodenum)
{
	//for (int i = 0; i < nodenum; i++) 
	//for (uint j: nodes) 
	for (uint j = 0; j < nodenum; ++j)
	{
		for (uint k : g[j].from)
		{
			/*if (k == MAXNUM)
				continue;*/
			for (uint i : g[k].from)
			{
				if (i != j)
					path_2[j][i].push_back(k);
			}
		}
	}

	//for (auto iter = path_2.begin(); iter != path_2.begin(); ++iter)
	//{
	//	for (auto &it : *iter)
	//		sort(it.second.begin(), it.second.end()); //将i->k_i->j中的k_i按序排好
	//}


	//for (int i = 0; i < nodenum; i++)
	//for (auto iter = path_2.begin() + st; iter != path_2.begin() + end; ++iter)
	//{
	//	for (auto &it : *iter)
	//		sort(it.second.begin(), it.second.end()); //将i->k_i->j中的k_i按序排好
	//}
	//for (auto iter = path_3.begin() + st; iter != path_3.begin() + end; ++iter)
	//{
	//	for (auto &it : *iter)
	//	{
	//		sort(it.second.begin(), it.second.end()); //u->i->k_i->j中的i按序排好
	//		it.second.erase(unique(it.second.begin(), it.second.end()), it.second.end());
	//	}
	//}
}

//void findCycle(uint v, uint layer, uint n)
//{
//	/*
//	v: 当前遍历到的顶点编号
//	layer: 当前是DFS的第几层。由main函数顶层调用时应设layer=1. layer最大为7。
//	*/
//	auto& endi = endind[n];
//	auto& tr = trace[n];
//	auto& vis = visit[n];
//	uint c = cur[n];
//
//	++endi;
//	tr[endi] = v;
//	vis[v] = 2;
//	int len; //环的长度
//	auto end = g[v].to.end();
//
//	if (layer < 5)
//	{
//		auto it = lower_bound(g[v].to.begin(), end, c);
//		if (it != end && *it == c && endi >= 2)
//		{
//			len = endi + 1;
//			//mu.lock();
//			res[n][len - 3].emplace_back(tr, len);
//			//mu.unlock();
//			++it;
//		}
//		for (; it != end; ++it)
//		{
//			if (vis[*it] != 2)
//				findCycle(*it, layer + 1, n);
//		}
//		//for (uint vn : g[v].to)
//		//{
//		//	if (vn < cur)
//		//		continue;       //这2句很重要
//		//	if (vn == cur && endind >= 2) //找到环了:3,4元环
//		//	{
//		//		len = endind + 1;
//		//		res[len - 3].emplace_back(trace, len);
//		//	}
//		//	else if (g[vn].visit != 2)
//		//		findCycle(vn, layer + 1);
//		//}
//	}
//
//	else //第5层
//	{
//		len = 5;
//		auto it = lower_bound(g[v].to.begin(), end, c);
//		if (it != end && *it == c)
//		{
//			//mu.lock();
//			res[n][len - 3].emplace_back(tr, len);
//			//mu.unlock();
//		}
//		//for (uint vn : g[v].to)
//		//{
//		//	if (vn == cur) //5元环
//		//	{
//		//		res[len - 3].emplace_back(trace, len);
//		//		break;
//		//	}
//		//}
//
//		len = 6;
//		if (path2_reachable[n][v] == 1)
//		{
//			++endi;
//			for (uint k : path_2[c][v])
//			{
//				if (vis[k] != 2) //6元环
//				{
//					tr[endi] = k;
//					//mu.lock();
//					res[n][len - 3].emplace_back(tr, len);
//					//mu.unlock();
//				}
//			}
//			--endi;
//		}
//
//		len = 7;
//		if (path3_reachable[n][v] == 1)
//		{
//			++endi;
//			//vector<unordered_map<uint, vector<uint>>> path_3;
//			//for (auto &ele : path_3[cur][v])
//			for (uint i : path_3[c][v])
//			{
//				if (vis[i] == 2)
//					continue;
//				tr[endi] = i;
//				//for (uint u : ele.second)
//				for (uint k : path_2[c][i])
//				{
//					if (vis[k] == 2)
//						continue;
//					tr[endi + 1] = k;
//					//mu.lock();
//					res[n][len - 3].emplace_back(tr, len);
//					//mu.unlock();
//				}
//			}
//			--endi;
//		}
//	}
//
//	vis[v] = 1;
//	--endi;
//}

void findCycle(uint v)
{
	trace[0] = v;
	visit[v] = 1;
	map<uint, vector<uint>>::iterator u2_it = path_2[v].lower_bound(v);
	auto u2_it_end = path_2[v].end();
	vector<uint>::iterator u1_it_st; //奇数层为uint, 偶数层为map iterator
	vector<uint>::iterator u3_it_st;
	vector<uint>::iterator u5_it_st;

	//第1层循环
	for (; u2_it != u2_it_end; ++u2_it) //u2_it:往上第2层。v自己是第0层。
	{
		u1_it_st = upper_bound(u2_it->second.begin(), u2_it->second.end(), v);
		auto u3_cycle = lower_bound(g[u2_it->first].from.begin(), g[u2_it->first].from.end(), v);
		if (*u3_cycle == v) //3元环
		{
			trace[1] = u2_it->first;
			for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
			{
				trace[2] = *u1_it;
				res[3].emplace_back(trace, 3);
			}
		}

		//往下之前：所有向上第1 2层的都设为visit. 后面还要修改回来
		for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
			++visit[*u1_it];
		++visit[u2_it->first];

		//第4层
		auto u4_it = path_2[u2_it->first].lower_bound(v); //往上第4层。v自己是第0层。
		if (u4_it->first == v) //4元环
		{
			trace[2] = u2_it->first;
			u3_it_st = upper_bound(u4_it->second.begin(), u4_it->second.end(), v);
			for (auto u3_it = u3_it_st; u3_it != u4_it->second.end(); ++u3_it)
			{
				if (visit[*u3_it] == 1)
					continue;
				trace[1] = *u3_it;
				for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
				{
					trace[3] = *u1_it;
					res[4].emplace_back(trace, 4);
				}
			}
			++u4_it;
		}


		//第2层循环
		for (; u4_it != path_2[u2_it->first].end(); ++u4_it)
		{
			if (visit[u4_it->first])
				continue;
			u3_it_st = upper_bound(u4_it->second.begin(), u4_it->second.end(), v);
			auto u5_cycle = lower_bound(g[u4_it->first].from.begin(), g[u4_it->first].from.end(), v);
			if (*u5_cycle == v) //5元环
			{
				trace[1] = u4_it->first;
				trace[3] = u2_it->first;
				for (auto u3_it = u3_it_st; u3_it != u4_it->second.end(); ++u3_it)  
				{
					if (visit[*u3_it] > 0)
						continue;
					trace[2] = *u3_it;
					for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
					{
						trace[4] = *u1_it;
						res[5].emplace_back(trace, 5);
					}
				}
			}


			//往下之前：所有向上第3 4层的都设为visit. 后面还要修改回来
			for (auto u3_it = u3_it_st; u3_it != u4_it->second.end(); ++u3_it)
				++visit[*u3_it];
			++visit[u4_it->first];


			//第6层
			auto u6_it = path_2[u4_it->first].lower_bound(v); //往上第6层。v自己是第0层。
			if (u6_it->first == v) //6元环
			{
				trace[2] = u4_it->first;
				trace[4] = u2_it->first;
				u5_it_st = upper_bound(u6_it->second.begin(), u6_it->second.end(), v);
				for (auto u5_it = u5_it_st; u5_it != u6_it->second.end(); ++u5_it)
				{
					if (visit[*u5_it] > 0)
						continue;
					trace[1] = *u5_it;
					for (auto u3_it = u3_it_st; u3_it != u4_it->second.end(); ++u3_it)
					{
						if (visit[*u3_it] > 1)
							continue;
						trace[3] = *u3_it;
						for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
						{
							trace[5] = *u1_it;
							res[6].emplace_back(trace, 6);
						}
					}
				}
				++u6_it;
			}

			//第3层循环
			for (; u6_it != path_2[u4_it->first].end(); ++u6_it) 
			{
				if (visit[u6_it->first])
					continue;
				u5_it_st = upper_bound(u6_it->second.begin(), u6_it->second.end(), v);
				auto u7_cycle = lower_bound(g[u6_it->first].from.begin(), g[u6_it->first].from.end(), v);
				if (*u7_cycle == v) //7元环
				{
					trace[1] = u6_it->first;
					trace[3] = u4_it->first;
					trace[5] = u2_it->first;
					for (auto u5_it = u5_it_st; u5_it != u6_it->second.end(); ++u5_it)
					{
						if (visit[*u5_it] > 0)
							continue;
						trace[2] = *u5_it;
						for (auto u3_it = u3_it_st; u3_it != u4_it->second.end(); ++u3_it)
						{
							if (visit[*u3_it] > 1)
								continue;
							trace[4] = *u3_it;
							for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
							{
								trace[6] = *u1_it;
								res[7].emplace_back(trace, 7);
							}
						}
					}
				}
			}

			for (auto u3_it = u3_it_st; u3_it != u4_it->second.end(); ++u3_it)
				--visit[*u3_it];
			--visit[u4_it->first];
		}

		for (auto u1_it = u1_it_st; u1_it != u2_it->second.end(); ++u1_it)
			--visit[*u1_it];
		--visit[u2_it->first];
	}

	visit[v] = 0;
}


void solve(uint st, uint end, uint n) //num: 编号
{
	//auto& p2 = path2_reachable;
	//auto& p3 = path3_reachable[n];
#ifdef DEBUG_thDFS
	int cnt = 1;
#endif // DEBUG

	for (uint i = st; i < end; ++i)
	{
		if (g[i].to_num == 0) //该节点不存在
			continue;
		//vector<unordered_map<uint, vector<uint>>> path_2;
		//for (auto& jks : path_2[i]) //2级可达
		//{
		//	p2[jks.first] = 1;
		//	in_path2.push_back(jks.first);
		//}
		//for (auto& uks : path_3[i])  //3级可达
		//{
		//	p3[uks.first] = 1;
		//	in_path3[n].push_back(uks.first);
		//}

		//cur = i;
		findCycle(i);
		//g[cur].visit = 3;

		/*for (uint t : in_path2[n])
			p2[t] = 0;
		in_path2[n].clear();
		for (uint t : in_path3[n])
			p3[t] = 0;
		in_path3[n].clear();*/

#ifdef DEBUG_thDFS
		if (cnt % 200 == 0)
			printf("%d-th DFS succeeded.\n", cnt);
		++cnt;
#endif // DEBUG

	}
}

//void solve_process(uint st, uint end, uint n)
//{
//	build_path_index(st, end);
//	solve(st, end, n);
//}

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
	{    /*
		if (g[i].to_num == 0)
		{
			toque.push(i);
			
		}
			*/
		if (g[i].from_num == 0)
			fromque.push(i);
		else
		{
			if (g[i].to_num == 0)
				toque.push(i);
			sort(g[i].from.begin(), g[i].from.end()); //顺便给指向的节点排序
		}
	}
	uint num;
	vector<uint> *vtmp;
	vector<uint>::iterator it;
	int in_cut = 0;
	int out_cut = 0;

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
				it = lower_bound(vtmp->begin(), vtmp->end(), num);
				it = vtmp->erase(it);
				//for (it = vtmp->begin(); it != vtmp->end(); ++it)
				//{
				//	if (*it == num)
				//	{
				//		//it = vtmp->erase(it);
				//		*it = MAXNUM;
				//		break;
				//	}
				//}
			}
		}

		//图中删除这一节点
		g[num].to_num = 0; //连clear都不用，到时候直接判断tonum是不是0就判断此点是否存在
		++in_cut;
	}
#ifdef DEBUG
	printf("Cut %u 0-in-degree nodes.\n", in_cut);
#endif // DEBUG

	//递归删除出度为0的节点
	while (!toque.empty())
	{
		num = toque.front();
		toque.pop();
		//在图中删除这一点：其所有父节点出度-1，父节点的孩子列表中删除该节点，若减到0则加入toque
		for (uint t : g[num].from)
		{
			/*if (t == MAXNUM)
				continue;*/
			--g[t].to_num;
			if (g[t].to_num == 0)
				toque.push(t);
			//else
			//{
			//	vtmp = &g[t].to;
			//	it = lower_bound(vtmp->begin(), vtmp->end(), num);
			//	it = vtmp->erase(it);
			//	/*for (it = vtmp->begin(); it != vtmp->end(); ++it)
			//	{
			//	if (*it == num)
			//	{
			//	it = vtmp->erase(it);
			//	break;
			//	}
			//	}*/
			//}
		}
		++out_cut;

#ifdef DEBUG
	printf("Cut %u 0-out-degree nodes.\n", out_cut);
	printf("Node num Trimmed=%u\n", g.size());
#endif // DEBUG
		//图中删除这一节点
		//g[num].to.clear();
	}
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
	path_2.resize(nodenum);
	visit.resize(nodenum, 0);
	for (uint i = 3; i <= 7; ++i)
		res[i].reserve(5000);
	//path_3.resize(nodenum);
}

void DFS()
{
	//uint n1, n2, n3, n4/*, n5, n6, n7, n8*//*, n9, n10*/;
	//n1 = nodenum / 8;
	//n2 = n1 * 2;
	//n3 = n1 * 4;
	//n4 = nodenum;
	///*n5 = n1 * 12;
	//n6 = n1 * 16;
	//n7 = n1 * 21;
	//n8 = nodenum;*/
	///*n9 = n1 * 27;
	//n10 = nodenum;*/
	//thread th0(solve_process, 0, n1, 0);
	//thread th1(solve_process, n1, n2, 1);
	//thread th2(solve_process, n2, n3, 2);
	//thread th3(solve_process, n3, n4, 3);
	///*thread th4(solve_process, n4, n5, 4);
	//thread th5(solve_process, n5, n6, 5);
	//thread th6(solve_process, n6, n7, 6);
	//thread th7(solve_process, n7, n8, 7);*/
	///*thread th8(solve_process, n8, n9, 8);
	//thread th9(solve_process, n9, n10, 9);*/
	//th0.join();
	//th1.join();
	//th2.join();
	//th3.join();
	/*th4.join();
	th5.join();
	th6.join();
	th7.join();*/
	/*th8.join();
	th9.join();*/
}

void write_file()
{
	uint sum = 0;/*
	for (uint i = 0; i < TNUM; ++i)
	{*/
		for (uint j = 3; j <= 7; ++j)
		{
			sum += res[j].size();
			sort(res[j].begin(), res[j].end());
		}
	//}

	//写入文件中
	vector<string> idstr(nodenum);
	for (uint i = 0; i < nodenum; ++i)
		//idstr.emplace(i, to_string(nodes[i]);
		idstr[i] = to_string(nodes[i]);

	FILE *fw = fopen(writepath, "w");
	fprintf(fw, "%u\n", sum); //先写入环的总数
	char strtmp[128];
	int len;
	for (uint i = 0; i < TNUM; ++i)
	{
		for (Cycle& c : res[3])
		{
			len = sprintf(strtmp, "%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str());
			fwrite(strtmp, len, 1, fw);
		}
	}

	for (uint i = 0; i < TNUM; ++i)
	{
		for (Cycle& c : res[4])
		{
			len = sprintf(strtmp, "%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str());
			fwrite(strtmp, len, 1, fw);
		}
	}

	for (uint i = 0; i < TNUM; ++i)
	{
		for (Cycle& c : res[5])
		{
			len = sprintf(strtmp, "%s,%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str(), idstr[c.arr[4]].c_str());
			fwrite(strtmp, len, 1, fw);
		}
	}

	for (uint i = 0; i < TNUM; ++i)
	{
		for (Cycle& c : res[6])
		{
			len = sprintf(strtmp, "%s,%s,%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str(), idstr[c.arr[4]].c_str(), idstr[c.arr[5]].c_str());
			fwrite(strtmp, len, 1, fw);
		}
	}

	for (uint i = 0; i < TNUM; ++i)
	{
		for (Cycle& c : res[7])
		{
			len = sprintf(strtmp, "%s,%s,%s,%s,%s,%s,%s\n", idstr[c.arr[0]].c_str(), idstr[c.arr[1]].c_str(), idstr[c.arr[2]].c_str(), idstr[c.arr[3]].c_str(), idstr[c.arr[4]].c_str(), idstr[c.arr[5]].c_str(), idstr[c.arr[6]].c_str());
			fwrite(strtmp, len, 1, fw);
		}
	}

	fclose(fw);
}

//============================================方法结束=================================================

int main()
{
#ifdef DEBUG_TIME
	clock_t timestart, timeend;
	timestart = clock();
	clock_t read_st, read_end;
	read_st = timestart;
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
	//int cnt = 1;
	init();
	build_path_index(nodenum);
	
#ifdef DEBUG_thDFS
	printf("DFS started.\n");
#endif // DEBUG
	//DFS();
	solve(0, nodenum, 1);

#ifdef DEBUG
	printf("DFS succeeded.\n");
#endif // DEBUG
#ifdef DEBUG_TIME
	builddfsend = clock();
#endif


	//排序，求总和:多线程下需排序
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

	//exit(0);
	//abort();
	return 0;
}