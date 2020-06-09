#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;
typedef unsigned int uint;

struct Vertices
{
	vector<uint> to; //每个节点指向的节点的列表，出度
	vector<uint> from;  //每个节点被哪些节点指向的列表，入度。用于拓扑排序中递归清除出度为0的节点
	int to_num = 0; //节点自己的出度，应保证=to.size()
	int from_num = 0; //节点自己的入度，应保证=from.size()
	int visit = 0;      //遍历用的变量，0代表完全未见过，1代表见过但未遍历完其所有7层后代
	//2代表正处于某个DFS中, 3代表该节点完全7层后代都被看过
};
unordered_map<uint, Vertices> g; //图
//FILE *fd = fopen("debug.txt", "w");

struct Cycle
{
	uint arr[7]; //存环的各个节点
	int size; //存环的大小：3-7
	Cycle(uint a[], int s)
	{
		size = s;
		int minind = 0;
		int i, j;
		for (i = 1; i < s; ++i)
		{
			//arr[i] = a[i];
			if (a[i] < a[minind])
				minind = i;
		}

		j = 0;
		for (i = minind; i < s; ++i)
		{
			arr[j] = a[i];
			++j;
		}
		for (i = 0; i < minind; ++i)
		{
			arr[j] = a[i];
			++j;
		}

		//fprintf(fd, "len=%d, st=%d, end=%d, ast=%d, aend=%d\n", s, arr[0], arr[s - 1], a[0], a[s-1]);
	}

	bool operator < (const Cycle& r)
	{
		//同样大小：按字典序来排
		for (int i = 0; i < size; ++i)
		{
			if (arr[i] < r.arr[i])
				return true;
			else if (arr[i] > r.arr[i])
				return false;
		}
		return false;
	}
};
uint trace[8]; //环缓存
int endind = -1;    //环缓存的最后位置的下标，也即环缓存为1--endind，endind <= 7
vector<Cycle> res[5]; //3 4 5 6 7个节点的环

inline int get_trace(uint v)
{
	for (int i = 0; i < endind; ++i)
	{
		if (trace[i] == v)
			return i;
	}
	return -1;
}

void findCycle(uint v, int layer)
{
	/*
	v: 当前遍历到的顶点编号
	layer: 当前是DFS的第几层。由main函数顶层调用时应设layer=1. layer最大为7。
	*/
	int ind;
	int len; //环的长度
	int visit = g[v].visit;
	if (visit == 2) //可能遇到环
	{
		ind = get_trace(v);
		if (ind != -1) //确实是环
		{
			len = endind - ind;
			if (len >= 3) //一定<=7，故无需额外判断
			{
				res[len - 3].push_back(Cycle(trace + ind, len));

				//printf("Added 1 cycle.\n");
			}

		}
		//return; //这条路探索完毕，不存在仍需要继续DFS递归的情况，因为v=2
	}

	else if (layer < 8)
	{
		g[v].visit = 2;
		++endind;
		for (uint vn : g[v].to)
		{
			if (g[vn].visit == 3)
				continue;       //这2句很重要
			trace[endind] = vn; //这2句很重要
			findCycle(vn, layer + 1);
		}
		--endind;
		g[v].visit = 1;
	}
}

const int SL = 128;
const char readpath[] = "test_data_big.txt"; //线上提交前：这里的路径要修改
const char writepath[] = "my_result_big.txt";

int main()
{
	//读文件
	clock_t timestart, timeend;
	timestart = clock();
	FILE *fp = fopen(readpath, "r"); 
	char linetmp[SL];
	char *digittmp;
	uint num1, num2;
	do 
	{
		fgets(linetmp, SL, fp);
		//printf(linetmp);
		//printf("%c", linetmp[0]);
		if (linetmp[0] == '\r' || linetmp[0] == '\n') continue;
		digittmp = strtok(linetmp, ","); //该函数线程不安全，如果用多线程读文件则用strtok_r(Linux)
		num1 = strtoul(digittmp, NULL, 10);
		digittmp = strtok(NULL, ","); //该函数线程不安全，如果用多线程读文件则用strtok_r(Linux)
		num2 = strtoul(digittmp, NULL, 10);
		g[num1].to.push_back(num2);
		g[num2].from.push_back(num1);
		++g[num1].to_num;
		++g[num2].from_num;
	} while (!feof(fp));
	printf("Read succeeded.\n");
	

	//拓扑排序来递归删除入度和出度为0的节点
	//首先得遍历一遍图，将当前所有入度和出度为0的入队列
	queue<uint> toque; //出度为0的点的队列
	queue<uint> fromque; //入度为0的点的队列
	for (unordered_map<uint, Vertices>::iterator it = g.begin(); it != g.end(); ++it)
	{
		if (it->second.to_num == 0) //出度为0
			toque.push(it->first);
		else if (it->second.from_num == 0) //入度为0
			fromque.push(it->first);
	}

	//递归删除入度为0的节点
	uint num;
	vector<uint> *vtmp;
	unordered_map<uint, Vertices>::iterator iter;
	while (!fromque.empty())
	{
		num = fromque.front();
		fromque.pop();
		//在图中删除这一点：其所有子节点入度-1，子节点的父亲列表中删除该节点，若减到0则加入fromque
		//vector<int> *from;
		for (uint t : g[num].to)
		{
			--g[t].from_num;
			vtmp = &g[t].from;
			for (vector<uint>::iterator it = vtmp->begin(); it != vtmp->end(); ++it)
			{
				if (*it == num)
				{
					it = vtmp->erase(it);
					break;
				}
			}

			if (g[t].from_num == 0)
				fromque.push(t);
		}

		//图中删除这一节点
		iter = g.find(num); 
		iter = g.erase(iter); 
	}
	printf("Cut 0-in-degree succeeded.\n");
	//
	while (!toque.empty())
	{
		num = toque.front();
		toque.pop();
		//在图中删除这一点：其所有父节点出度-1，父节点的孩子列表中删除该节点，若减到0则加入toque
		for (uint t : g[num].from)
		{
			--g[t].to_num;
			vtmp = &g[t].to;
			for (vector<uint>::iterator it = vtmp->begin(); it != vtmp->end(); ++it)
			{
				if (*it == num)
				{
					it = vtmp->erase(it);
					break;
				}
			}

			if (g[t].to_num == 0)
				toque.push(t);
		}

		//图中删除这一节点
		iter = g.find(num);
		iter = g.erase(iter);
	}
	printf("Cut 0-out-degree succeeded.\n");


	//开始DFS
	int cnt = 1;

	for (iter = g.begin(); iter != g.end(); ++iter)
	{
		if (iter->second.visit != 3)
		{
			num = iter->first;
			++endind;
			trace[endind] = num; //这句很重要
			findCycle(num, 1);
			--endind;
			g[num].visit = 3;

			printf("%d-th DFS succeeded.\n", cnt);
			++cnt;
		}
	}
	printf("DFS succeeded.\n");


	//排序，求总和
	uint sum = 0;
	for (int i = 0; i < 5; ++i)
	{
		sort(res[i].begin(), res[i].end());
		sum += res[i].size();
	}

	//写入文件中
	FILE *fw = fopen(writepath, "w");
	fprintf(fw, "%u\n", sum); //先写入环的总数
	for (int i = 0; i < 5; ++i)
	{
		for (Cycle& c : res[i])
		{
			int j = 0;
			for (; j < c.size - 1; ++j)
				fprintf(fw, "%u,", c.arr[j]);
			fprintf(fw, "%u\n", c.arr[j]);
		}
	}
	//fprintf(fw, "")
	
	timeend = clock();
	printf("Total time: %f", (double)(timeend - timestart) / CLOCKS_PER_SEC);
	fclose(fw);
	fclose(fp);
	return 0;
}