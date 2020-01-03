#!/usr/bin/python3
'''
Dependency: Graphviz, install by the following commands:
`brew install graphviz`
`pip3 install graphviz`
'''
import os
import re
import argparse
import codecs
from collections import defaultdict
from graphviz import Digraph

include_regex = re.compile('#include\s+["<"](.*)[">]')
valid_extensions = ['.cc', '.h']

def normalize(path):
	filename = os.path.basename(path)
	end = filename.rfind('.')
	end = end if end != -1 else len(filename)
	return filename[:end]

def get_extension(path):
	return path[path.rfind('.'):]

def find_all_files(path, recursive=True):
	files = []
	for entry in os.scandir(path):
		if entry.is_dir() and recursive:
			files += find_all_files(entry.path)
		elif get_extension(entry.path) in valid_extensions:
			files.append(entry.path)
	return files

def find_neighbors(path):
	f = codecs.open(path, 'r', "utf-8", "ignore")
	code = f.read()
	f.close()
	return [normalize(include) for include in include_regex.findall(code)]

def create_graph(folder, create_cluster, strict):
	files = find_all_files(folder)
	folder_to_files = defaultdict(list)
	for path in files:
		folder_to_files[os.path.dirname(path)].append(path)
	nodes = {normalize(path) for path in files}
	graph = Digraph(strict=strict)
	for folder in folder_to_files:
		with graph.subgraph(name='cluster_{}'.format(folder)) as cluster:
			for path in folder_to_files[folder]:
				node = normalize(path)
				if create_cluster:
					cluster.node(node)
				else:
					graph.node(node)
				neighbors = find_neighbors(path)
				for neighbor in neighbors:
					if neighbor != node and neighbor in nodes:
						graph.edge(node, neighbor)
	return graph

if __name__ == '__main__':
	parser = argparse.ArgumentParser()
	parser.add_argument('folder', help='Path to the folder to scan.')
	parser.add_argument('output', help='Path of the output file without the extension.')
	parser.add_argument('-v', '--view', action='store_true', help='View the graph after generating.')
	parser.add_argument('-c', '--cluster', action='store_true', help='Create a cluster for each subfolder.')
	parser.add_argument('-s', '--strict', action='store_true', help='Rendering should merge multi-edges.', default=False)
	args = parser.parse_args()
	graph = create_graph(args.folder, args.cluster, args.strict)
	graph.format = 'pdf'
	graph.render(args.output, cleanup=True, view=args.view)
