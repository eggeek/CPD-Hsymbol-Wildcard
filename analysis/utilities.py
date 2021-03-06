import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import json
import os

alias = {}
styles = {}

def parse_index_name(iname):
    # parse '*.map-DFS-3-c4-inv' to itype="inv", d=4, hlevel=3
    # or '*.map-DFS-3-c4' to itype="fwd", d=4, hlevel=3
    ps = iname.split('-')
    itype = "forward"
    if ps[-1] == "inv":
        itype = "backward"
        ps.pop()
    r = int(ps[-1][1:]) if ps[-1][0] == 'c' else 0
    ps.pop()
    ps.pop()
    ps.pop()
    mname = '-'.join(ps)[:-4]
    return dict(map=mname,itype=itype,r=r)

def load_size(fname, filter=True):
    with open(fname, 'r') as f:
        raw = f.readlines()
    rows = []
    game_maps = os.listdir("../maps/gppc")
    header = ["map", "itype", "r", "size"]
    for line in raw:
        size, path = line.split()
        data = parse_index_name(path)
        mapname = data['map'].split('/')[-1]
        if not filter or (mapname + ".map") in game_maps: 
            rows.append([mapname, data['itype'], data['r'], size])
    df = pd.DataFrame.from_records(rows, columns=header)
    df = df[header].apply(pd.to_numeric, errors="ignore")
    return df

def load_data(fname):
    with open(fname, "r") as f:
        # ignore `vertices: ...` at head and `Total: ..` at tail
        raws = f.readlines()
    header = [i.strip() for i in raws[0].split(',')]
    lines = [[j.strip() for j in i.split(',')] for i in raws[1:]]
    t = pd.DataFrame.from_records(lines, columns=header)
    res = t[header].apply(pd.to_numeric, errors='ignore')
    return res

def load_files(paths):
    frames = []
    for i in paths:
        print(i)
        frames.append(load_data(i))
    res = pd.concat(frames)
    return res

def gen_xy(df=None, colx='', coly='', ignore=True, limit=20, aggregate=np.average):
    tg = df.groupby(colx)
    x = []
    y = []
    for k, v in tg[coly].apply(lambda _: aggregate(_)).items():
        if ignore and tg.size()[int(k)] < limit:
            continue
        x.append(int(k))
        y.append(v)
    return x, y

def plot_graph(xlabel='', ylabel='', xs=[[]], ys=[[]], labels=[], color=None, 
               yscale='log', xscale=None, ylim=None, xlim=None, saveto=None, xticks=None, yticks=None,
               loc='out', **kw):
    
    fig, ax = plt.subplots()
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_yscale(yscale)
    font = {'family': 'monospace',
            'weight': 'bold',
            'size': 22}
    plt.rc('font',**font)
    plt.rc("text", usetex=True)
    if xscale is not None:
      ax.set_xscale(xscale)
    if ylim is not None:
        ax.set_ylim(ylim)
    if xlim is not None:
        ax.set_xlim(xlim)
    if xticks is not None:
      print(xticks)
      plt.xticks(xticks)
    if yticks is not None:
      plt.yticks(yticks)

    n = len(xs)
    for i in range(n):
        x = xs[i]
        y = ys[i]
        if styles.get(labels[i]) is not None:
            plt.plot(x, y, styles[labels[i]], label=labels[i], markersize=5)
        else:
            # ax.scatter(x, y)
            ax.plot(x, y, label=labels[i])
    ax.legend(labels)
    if loc == 'in':
      ax.legend(loc='best', fancybox=True, framealpha=0, prop={'size': 14})
    else:
      bbox_to_anchor = kw.get("bbox_to_anchor", (0.5, 1.4))
      ax.legend(loc='upper center', bbox_to_anchor=bbox_to_anchor, ncol=4, fancybox=True, prop={'size': 16})
    plt.grid(True)
    if saveto is not None:
        fig.savefig(saveto, bbox_inches='tight')
