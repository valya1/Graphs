#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<queue>
#include <functional>
#include<algorithm>
#include<sstream>
#include<fstream>

using namespace std;

#define UNDEFINED -1

class StringSplitter {
public:
	static vector<string> split(string str, char c) {
		stringstream stream(str);
		string segment;
		vector<string> result;
		while (getline(stream, segment, c)) result.push_back(segment);
		return result;
	}
};

class DSU {
private :
	vector<int> parent;
	vector<int> size;
public:
	DSU() {	}
	DSU(int n) {
		parent.resize(n);
		size.resize(n);
	}
	~DSU() {}
	
	void makeSet(int v) {
		parent[v] = v;
		size[v] = 1;
	}

	int find(int x) {
		if (parent[x] == x)
			return x;
		int xRoot = find(parent[x]);
		parent[x] = xRoot;
		return xRoot;
	}

	void unite(int x, int y) {
		int xRoot = find(x);
		int yRoot = find(y);
		if (xRoot != yRoot) {
			if (size[xRoot] >= (size[yRoot])){
				size[xRoot] += size[yRoot];
				size[yRoot] = xRoot;
			}
			else{
				size[yRoot] += size[xRoot];
				size[xRoot] = yRoot;
			}
		}
	}

	map<int, int> getRoots() {
		map<int, int> roots = map<int, int>();
		for (int i = 0; i < parent.size(); ++i)
			if (i == parent[i])
				roots[i] = size[i];
		return roots;
	}
};


class BaseGraphRepresentation {

public:

	int mVertexAmount;
	int mEdgesAmount;
	bool mIsWeighted;
	bool mIsOriented;

	BaseGraphRepresentation(int vertexAmount, bool isOriented, bool isWeighted) {
		mVertexAmount = vertexAmount;
		mEdgesAmount = UNDEFINED;
		mIsOriented = isOriented;
		mIsWeighted = isWeighted;
	}

	BaseGraphRepresentation(int vertexAmount, int edgesAmount, bool isOriented, bool isWeighted) {
		mVertexAmount = vertexAmount;
		mEdgesAmount = edgesAmount;
		mIsOriented = isOriented;
		mIsWeighted = isWeighted;
	}

	virtual ~BaseGraphRepresentation() = default;
	virtual void addEdge(int from, int to, int weight) = 0;
	virtual void removeEdge(int from, int to) = 0;
	virtual int changeEdge(int from, int to, int newWeight) = 0; //возвращает старое значение веса ребра
	virtual void writeGraph(ostream& stream) = 0;
	virtual BaseGraphRepresentation* transformToAdjList() = 0;
	virtual BaseGraphRepresentation* transformToAdjMatrix() = 0;
	virtual BaseGraphRepresentation* transformToListOfEdges() = 0;
};

class AdjMatrixGraphRepresentation : public BaseGraphRepresentation {

private:

	void readMatrix(istream& stream, int vertexAmount) {
		string line;
		for (size_t i = 0; i < vertexAmount; i++) {
			adjMatrix.push_back(vector<int>());
			getline(stream, line);
			for (auto weight : StringSplitter::split(line, ' ')) {
				adjMatrix[i].push_back(stoi(weight));
			}
		}
	}

	void transformTo(BaseGraphRepresentation* target) {
		for (size_t i = 0; i < adjMatrix.size(); ++i)
		{
			for (size_t j = 0; j < adjMatrix.size(); ++j)
			{
				int weight = adjMatrix[i][j];
				if (weight > 0) target->addEdge(i, j, weight);
			}
		}
	}

public:

	vector < vector < int > > adjMatrix;

	AdjMatrixGraphRepresentation(istream &stream, int vertexAmount, bool isOriented, bool isWeighted)
		: BaseGraphRepresentation(vertexAmount, isOriented, isWeighted) {
		readMatrix(stream, vertexAmount);
	}

	AdjMatrixGraphRepresentation(int vertexAmount, bool isOriented, bool isWeighted)
		: BaseGraphRepresentation(vertexAmount, isOriented, isWeighted) {
		for (size_t i = 0; i < vertexAmount; i++) {
			adjMatrix.push_back(vector<int>(vertexAmount));
		}
	}

	AdjMatrixGraphRepresentation(const AdjMatrixGraphRepresentation& graph)
		: BaseGraphRepresentation(graph.mVertexAmount, graph.mIsOriented, graph.mIsWeighted) {
		this->adjMatrix = graph.adjMatrix;
	}

	void addEdge(int from, int to, int weight) override {
		adjMatrix[from][to] = weight;
		if (!mIsOriented) adjMatrix[to][from] = weight;
	}

	void removeEdge(int from, int to) override {
		adjMatrix[from][to] = 0;
		if (!mIsOriented) adjMatrix[to][from] = 0;
	}

	int changeEdge(int from, int to, int newWeight) override {
		int result = adjMatrix[from][to];
		addEdge(from, to, newWeight);
		return result;
	}

	void writeGraph(ostream& stream) override {
		stream << "C " << mVertexAmount << endl;
		stream << (mIsOriented ? "1 " : "0 ") << (mIsWeighted ? "1" : "0") << endl;
		for (size_t i = 0; i < mVertexAmount; i++) {
			for (size_t j = 0; j < mVertexAmount; j++) {
				stream << adjMatrix[i][j];
				if (j != mVertexAmount - 1) stream << " ";
			}
			if (i != mVertexAmount - 1) stream << endl;

		}
	}

	BaseGraphRepresentation* transformToAdjMatrix() override { return new AdjMatrixGraphRepresentation(*this); }

	BaseGraphRepresentation* transformToAdjList() override;

	BaseGraphRepresentation* transformToListOfEdges() override;
};


class AdjListGraphRepresentation : public BaseGraphRepresentation {

private:

	void readList(istream& stream) {
		string line;
		vector<string> tokens;
		/*getline(stream, line);*/
		for (size_t i = 0; i < mVertexAmount; i++) {
			adjList.push_back(map<int, int>());
			getline(stream, line);
			tokens = StringSplitter::split(line, ' ');
			for (size_t j = 0; j < tokens.size(); j++) {
				if (!mIsWeighted) {
					adjList[i].insert(make_pair(stoi(tokens[j]) - 1, 1));
				}
				else {
					adjList[i].insert(make_pair(stoi(tokens[j - 1]) - 1, stoi(tokens[++j])));
				}
				//adjList[i].insert(make_pair(  здесь происходила магия с инкрементом, не работает
				//stoi(tokens[j-1]) - 1, //индекс вершины
				//mIsWeighted ? stoi(tokens[++j]) : 1 //вес вершины (если граф не взвешенный -> 1)
			}
		}
	}

	void transformTo(BaseGraphRepresentation* target) {
		for (size_t i = 0; i < adjList.size(); i++) {
			for (auto vertex : adjList[i]) {
				target->addEdge(i, vertex.first, vertex.second);
			}
		}
	}

	bool dfsKuhn(int v, vector<bool> &used, vector<int> &part) {
		if (used[v]) return false;
		used[v] = true;
		for (auto it = adjList[v].begin(); it != adjList[v].end(); it++) {
			int to = it->first;
			if (part[to] == -1 || dfsKuhn(part[to], used, part)) {
				part[to] = v;
				return true;
			}
		}
		return false;
	}

public:

	vector< map < int, int > > adjList;

	AdjListGraphRepresentation(
		istream &stream,
		int vertexAmount,
		bool isOriented,
		bool isWeighted)
		: BaseGraphRepresentation(vertexAmount, isOriented, isWeighted) {
		readList(stream);
	};

	AdjListGraphRepresentation(
		int vertexAmount,
		bool isOriented,
		bool isWeighted)
		: BaseGraphRepresentation(vertexAmount, isOriented, isWeighted) {
		adjList.resize(vertexAmount);
	}

	AdjListGraphRepresentation(AdjListGraphRepresentation& graph) 
		: BaseGraphRepresentation(graph.mVertexAmount, graph.mIsOriented, graph.mIsWeighted){
		this->adjList = graph.adjList;
	}

	void addEdge(int from, int to, int weight) override {
		if (from > adjList.size())
			adjList.resize(from);
		adjList[from].insert(make_pair(to, weight));
		if (!mIsOriented) {
			if (to > adjList.size())
				adjList.resize(to);
			adjList[to].insert(make_pair(from, weight));
		}
	}

	void removeEdge(int from, int to) override {
		adjList[from].erase(adjList[from].find(to));
		if (!mIsOriented) adjList[to].erase(adjList[to].find(from));
	}

	int changeEdge(int from, int to, int newWeight) override {
		int result = adjList[from][to];
		addEdge(from, to, newWeight);
		return result;
	}


	int checkBipart(vector<char> &marks) {
		marks = vector<char>(adjList.size(), -1);
		for (int i = 0; i < adjList.size(); i++) {
			if (marks[i] == -1) {
				queue<int> q = queue<int>();
				q.push(i);
				marks[i] = 'A';
				while (!q.empty()) {
					int v = q.front();
					q.pop();
					for (auto it = adjList[v].begin(); it != adjList[v].end(); it++) {
						int to = it->first;
						if (marks[to] == -1) {
							if (marks[v] == 'A')
								marks[to] = 'B';
							else
								marks[to] = 'A';
							q.push(to);
						}
						else if (marks[to] == marks[v]) return 0;
					}
				}
			}
		}
		return 1;
	}

	vector<pair<int, int>> getMaximumMatchingBipart() {
		vector<char> marks;
		vector<pair<int, int>> result;
		//if (checkBipart(marks)) {
			vector<int> part = vector<int>(mVertexAmount, -1);
			vector<bool> used = vector<bool>(mVertexAmount, false);
			for (int i = 0; i < mVertexAmount; i++) dfsKuhn(i, used, part);
			for (int i = 0; i < mVertexAmount; i++) {
				if (used[i] && part[i] != -1) {
					result.push_back(pair<int, int>(i + 1, part[i] + 1));
					used[i] = false;
					used[part[i]] = false;
				}
				//}
			}
		return result;
	}

	void writeGraph(ostream& stream) override {
		stream << "L " << mVertexAmount << endl;
		stream << (mIsOriented ? "1 " : "0 ") << (mIsWeighted ? "1" : "0") << endl;
		for (size_t i = 0; i < mVertexAmount; i++) {
			for (map<int, int>::iterator it = adjList[i].begin(); it != adjList[i].end(); it++) {
				stream << (*it).first + 1;
				if (mIsWeighted) stream << " "<< (*it).second;
				if (next(it) != adjList[i].end()) stream << " ";
			}
			if (i != mVertexAmount - 1) stream << endl;
		}
	}

	BaseGraphRepresentation* transformToAdjList() override { return new AdjListGraphRepresentation(*this); }

	BaseGraphRepresentation* transformToAdjMatrix() override;

	BaseGraphRepresentation* transformToListOfEdges() override;
};

class ListOfEdgesGraphRepresentation : public BaseGraphRepresentation {

public:

	map< pair< int, int >, int> edges;

	ListOfEdgesGraphRepresentation(
		istream &stream,
		int vertexAmount,
		int edgesAmount,
		bool isOriented,
		bool isWeighted)
		: BaseGraphRepresentation(vertexAmount, edgesAmount, isOriented, isWeighted) {
		readList(stream, edgesAmount);
	};

	ListOfEdgesGraphRepresentation(
		int vertexAmount,
		bool isOriented,
		bool isWeighted
	) : BaseGraphRepresentation(vertexAmount, 0, isOriented, isWeighted) { }

	ListOfEdgesGraphRepresentation(ListOfEdgesGraphRepresentation& graph)
		: BaseGraphRepresentation(graph.mVertexAmount, graph.mEdgesAmount, graph.mIsOriented, graph.mIsWeighted){
		this->edges = graph.edges;
	}

	void addEdge(int from, int to, int weight) override {
		if (!mIsOriented && edges.count(pair<int, int>(to, from)) != 0)
			return;
		mEdgesAmount++;
		edges[pair<int, int>(from, to)] = weight;
	}

	void removeEdge(int from, int to) override {
		mEdgesAmount--;
		edges.erase(pair<int, int>(from, to));
	}
	
	int changeEdge(int from, int to, int newWeight) {
		int result = edges[pair<int, int>(from, to)];
		edges[pair<int, int>(from, to)] = newWeight;
		return result;
	}

	void writeGraph(ostream& stream) {
		stream << "E " << mVertexAmount <<" "<< mEdgesAmount << endl;
		stream << (mIsOriented ? "1 " : "0 ") << (mIsWeighted ? "1" : "0") << endl;
		int i = 0;
		for (auto edge : edges) {
			i++;
			stream << edge.first.first + 1 << " " << edge.first.second + 1;
			if (mIsWeighted) stream << " " << edge.second;
			if(i <= edges.size()-1) stream << endl;
		}
	}

	BaseGraphRepresentation* transformToAdjList() override;
	
	BaseGraphRepresentation* transformToAdjMatrix() override;

	BaseGraphRepresentation* transformToListOfEdges() override { return new ListOfEdgesGraphRepresentation(*this); }

private:

	void readList(istream& stream, int edgesAmount) {
		string line;
		vector<string> tokens;
		int from, to, weight;
		for (size_t i = 0; i < edgesAmount; i++) {
			getline(stream, line);
			tokens = StringSplitter::split(line, ' ');
			from = stoi(tokens[0]) - 1;
			to = stoi(tokens[1]) - 1;
			weight = tokens.size() > 2 ? stoi(tokens[2]) : 1;
			edges[pair<int, int>(from, to)] = weight;
		}
	}

	void transformTo(BaseGraphRepresentation* target) {
		for (auto edge : edges) {
			target->addEdge(edge.first.first, edge.first.second, edge.second);
		}
	}

};


BaseGraphRepresentation* AdjMatrixGraphRepresentation::transformToAdjList() {
	BaseGraphRepresentation* result = new AdjListGraphRepresentation(mVertexAmount, mIsOriented, mIsWeighted);
	transformTo(result);
	return result;
}

BaseGraphRepresentation* AdjMatrixGraphRepresentation::transformToListOfEdges() {
	BaseGraphRepresentation* result = new ListOfEdgesGraphRepresentation(mVertexAmount, mIsOriented, mIsWeighted);
	transformTo(result);
	return result;
}


BaseGraphRepresentation* AdjListGraphRepresentation::transformToAdjMatrix() {
	BaseGraphRepresentation* result = new AdjMatrixGraphRepresentation(mVertexAmount, mIsOriented, mIsWeighted);
	transformTo(result);
	return result;
}

BaseGraphRepresentation* AdjListGraphRepresentation::transformToListOfEdges() {
	BaseGraphRepresentation* result = new ListOfEdgesGraphRepresentation(mVertexAmount, mIsOriented, mIsWeighted);
	transformTo(result);
	return result;
}



BaseGraphRepresentation* ListOfEdgesGraphRepresentation::transformToAdjList() {
	BaseGraphRepresentation* result = new AdjListGraphRepresentation(mVertexAmount, mIsOriented, mIsWeighted);
	transformTo(result);
	return result;
}

BaseGraphRepresentation* ListOfEdgesGraphRepresentation::transformToAdjMatrix(){
	BaseGraphRepresentation* result = new AdjMatrixGraphRepresentation(mVertexAmount, mIsOriented, mIsWeighted);
	transformTo(result);
	return result;
}


class Graph {
public: 

	Graph(){}

	Graph(int N) {
		graphRepresentation = new ListOfEdgesGraphRepresentation(N, false, true);
	}

	void readGraph(string fileName) {
		ifstream file(fileName);
		string paramsLine;
		getline(file, paramsLine);
		vector<string> lineTokens = StringSplitter::split(paramsLine,' ');
		char type = lineTokens[0][0];
		int vertexAmount = stoi(lineTokens[1]);
		int edgesAmount = lineTokens.size() > 2 ? stoi(lineTokens[2]) : UNDEFINED;
		getline(file, paramsLine);
		lineTokens = StringSplitter::split(paramsLine, ' ');
		bool isOriented = lineTokens[0] == "1";
		bool isWeighted = lineTokens[1] == "1";

		graphRepresentation = getGraphRepresentation(
			file,
			type,
			vertexAmount,
			edgesAmount,
			isOriented,
			isWeighted
		);
	}

	void addEdge(int from, int to, int weight) {
		graphRepresentation->addEdge(from-1, to-1, weight);
	}

	void removeEdge(int from, int to) {
		graphRepresentation->removeEdge(from-1, to-1);
	}

	int changeEdge(int from, int to, int newWeight) {
		return graphRepresentation->changeEdge(from-1, to-1, newWeight);
	}

	void writeGraph(string fileName) {
		//ofstream file(fileName);
		//graphRepresentation->writeGraph(file);
		graphRepresentation->writeGraph(cout);
		//file.close();
	}

	void transformToAdjList() {
		BaseGraphRepresentation* newGraph = graphRepresentation->transformToAdjList();
		delete graphRepresentation;
		graphRepresentation = newGraph;
	}

	void transformToAdjMatrix() {
		BaseGraphRepresentation* newGraph = graphRepresentation->transformToAdjMatrix();
		delete graphRepresentation;
		graphRepresentation = newGraph;
	}

	void transformToListOfEdges() {
		BaseGraphRepresentation* newGraph = graphRepresentation->transformToListOfEdges();
		delete graphRepresentation;
		graphRepresentation = newGraph;
	}


	Graph getSpaingTreePrima() {
		transformToAdjList();
		vector<map<int, int>> adjList = ((AdjListGraphRepresentation*)graphRepresentation)->adjList;
		priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
		int src = 0;
		vector<int> dists (adjList.size(), INT32_MAX);
		vector<int> parent(adjList.size(), -1);
		vector<bool> inMST(adjList.size(), false);
		
		pq.push(make_pair(0, src));
		while (!pq.empty()){
			int u = pq.top().second;
			pq.pop();
			inMST[u] = true;
			for (auto it = adjList.at(u).begin(); it != adjList.at(u).end(); ++it) {
				int v = it->first;
				int weight = it->second;
				if (inMST[v] == false && dists[v] > weight) {
					dists[v] = weight;
					pq.push(make_pair(dists[v], v));
					parent[v] = u;
				}
			}
		}
		ListOfEdgesGraphRepresentation* resultGraphRepresentation = new ListOfEdgesGraphRepresentation(adjList.size(), false, true);
		for (size_t i = 1; i < parent.size(); ++i)
			resultGraphRepresentation->addEdge(parent[i], i, dists[i]);
		Graph* graph = new Graph();
		graph->graphRepresentation = resultGraphRepresentation;
		return *graph;
	}

	Graph getSpaingTreeKruscal() {
		transformToListOfEdges();
		ListOfEdgesGraphRepresentation* edgeListGraph = (ListOfEdgesGraphRepresentation*)graphRepresentation;

		auto cmp = [](pair<pair<int, int>, int> a, pair<pair<int, int>, int> b) {
			return a.second < b.second;
		};
		auto edgeVector = vector<pair<pair<int, int>, int>>(edgeListGraph->edges.begin(), edgeListGraph->edges.end());
		sort(edgeVector.begin(), edgeVector.end(), cmp);

		DSU dsu = DSU(edgeListGraph->mVertexAmount);
		for (size_t i = 0; i < edgeListGraph->mVertexAmount; ++i)
			dsu.makeSet(i);

		ListOfEdgesGraphRepresentation* newEdgeListGraph = new ListOfEdgesGraphRepresentation(edgeListGraph->mVertexAmount, false, true);
		for (size_t i = 0; i < edgeVector.size(); i++)
		{
			int from = edgeVector[i].first.first;
			int to = edgeVector[i].first.second;
			if (dsu.find(from) != dsu.find(to)) {
				newEdgeListGraph->addEdge(from, to, edgeVector[i].second);
				dsu.unite(from, to);
			}
		}
		Graph* graph = new Graph();
		graph->graphRepresentation = newEdgeListGraph;
		return *graph;
	}

	Graph getSpaingTreeBoruvka() {
		transformToListOfEdges();
		ListOfEdgesGraphRepresentation* edgeListGraph = (ListOfEdgesGraphRepresentation*)graphRepresentation;
		map<pair<int,int>,int> edgeList = edgeListGraph->edges;
		int vertexAmount = edgeListGraph->mVertexAmount;
		auto edgeVector = vector<pair<pair<int, int>, int>>(edgeListGraph->edges.begin(), edgeListGraph->edges.end());

		DSU dsu = DSU(vertexAmount);
		for (size_t i = 0; i < edgeListGraph->mVertexAmount; ++i)
			dsu.makeSet(i);

		ListOfEdgesGraphRepresentation* newEdgeListGraph = new ListOfEdgesGraphRepresentation(vertexAmount, false, true);
		while (newEdgeListGraph->mEdgesAmount < vertexAmount - 1) {
			auto minEdges = map<int, int>();
			for (int i = 0; i < vertexAmount; ++i)
				minEdges[i] = -1;
			for (int i = 0; i < edgeVector.size(); ++i)
			{
				auto edge = edgeVector[i];
				int from = edge.first.first;
				int to = edge.first.second;
				int weight = edge.second;
				int fromComponent = dsu.find(from);
				int toComponent = dsu.find(to);
				if (fromComponent != toComponent) {
					if (minEdges[fromComponent] == -1 || edgeVector[minEdges[fromComponent]].second > weight)
						minEdges[fromComponent] = i;
					if (minEdges[toComponent] == -1 || edgeVector[minEdges[toComponent]].second > weight)
						minEdges[toComponent] = i;
				}
			}
			for (int i = 0; i < minEdges.size(); i++) {
				if (minEdges[i] != -1) {
					pair<pair<int, int>, int> edge = edgeVector[minEdges[i]];
					dsu.unite(edge.first.first, edge.first.second);
					newEdgeListGraph->addEdge(edge.first.first, edge.first.second, edge.second);
				}
			}
		}

		Graph* graph = new Graph();
		graph->graphRepresentation = newEdgeListGraph;
		return *graph;
	}

	vector<pair<int, int>> getMaximumMatchingBipart() {
		transformToAdjList();
		return ((AdjListGraphRepresentation*)graphRepresentation)->getMaximumMatchingBipart();
	}

	bool checkBipart(vector<char> &marks) {
		transformToAdjList();
		return ((AdjListGraphRepresentation*)graphRepresentation)->checkBipart(marks);
	}

private:
	BaseGraphRepresentation * graphRepresentation = 0;

	BaseGraphRepresentation* getGraphRepresentation(
		istream& stream,
		char type,
		int vertexAmount,
		int edgesAmount,
		bool isOriented,
		bool isWeighted
	) {
		switch (type) {
		case 'C': return new AdjMatrixGraphRepresentation(stream, vertexAmount, isOriented, isWeighted);
		case 'L': return new AdjListGraphRepresentation(stream, vertexAmount, isOriented, isWeighted);
		case 'E': return new ListOfEdgesGraphRepresentation(stream, vertexAmount, edgesAmount, isOriented, isWeighted);
		default: return 0;
		}
	}
};

int main()
{
	//Graph* graph = new Graph();
	//graph->readGraph("D:\\graph.txt");
	//graph->transformToAdjList();
	//graph->writeGraph("");
	//graph->transformToAdjMatrix();
	//graph->writeGraph("");
	//graph->transformToListOfEdges();
	//graph->writeGraph("");
	//graph->transformToAdjList();
	//graph->writeGraph("");
	//graph->transformToAdjMatrix();
	//graph->writeGraph("");
	//graph->transformToListOfEdges();
	//graph->transformToAdjList();
	//graph->writeGraph("");
	//graph->transformToAdjMatrix();
	//graph->writeGraph("");
	//graph->transformToListOfEdges();
	//graph->writeGraph("D:\\list_of_edges_to_adj_matrix_graph_result.txt");
	//graph->transformToAdjList();
	//graph->writeGraph("D:\\adj_list_graph_result.txt");
	//graph->transformToListOfEdges();
	//graph->writeGraph("D:\\lisf_of_edges_graph_result.txt");

	//Graph spaingGraph1 = graph->getSpaingTreeBoruvka();
	//spaingGraph1.transformToAdjMatrix();
	//spaingGraph1.writeGraph("D:\\Boruvka_result.txt");
	//Graph spaingGraph2 = graph->getSpaingTreeKruscal();
	//spaingGraph2.transformToAdjMatrix();
	//spaingGraph2.writeGraph("D:\\Kruskal_result.txt");
	//Graph spaingGraph3 = graph->getSpaingTreePrima();
	//spaingGraph3.transformToAdjMatrix();
	//spaingGraph3.writeGraph("D:\\Prima_result.txt");
 //   return 0;

	//freopen("output.txt", "w", stdout);
	Graph g;
	g.readGraph("D:\\graph.txt");
	g.transformToListOfEdges();
	std::vector<char> marks(1e5, ' ');
	if (!g.checkBipart(marks))
		std::cout << -1 << std::endl;
	else
	{
		std::vector<std::pair<int, int>>	vec = g.getMaximumMatchingBipart();
		std::cout << vec.size() << std::endl;
		for (auto i : vec)
			std::cout << i.first << " " << i.second << std::endl;
	}
	return 0;
}
