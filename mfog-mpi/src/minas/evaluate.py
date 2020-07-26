#!/usr/bin/python3
# coding: utf-8

import sys
import os.path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
# get_ipython().run_line_magic('matplotlib', 'inline')
# plt.close('all')

def getExamplesDf(path):
    assert os.path.isfile(path), "file '%s' not found." % path
    df = pd.read_csv(filepath_or_buffer=path, header=None)
    df['id'] = df.index
    df['class'] = df[22]
    return df

def getOriginalMatchesDf(path):
    assert os.path.isfile(path), "file '%s' not found." % path
    df = pd.read_table(filepath_or_buffer=path, header=None)
    df.columns = ['id', 'class', 'label']
    df = df[df['id'].str.startswith('Ex:')]

    def cleanLabel(text):
        label = text.replace('Classe MINAS:', '').strip()
        if label == 'Unk':
            return '-'
        if label.startswith('C '):
            return label.replace('C ', '')
        return label
    return pd.DataFrame({
        'id': df['id'].apply(lambda x: x.replace('Ex:', '').strip()).astype(int) - 1,
        'label': df['label'].apply(cleanLabel),
    })

def getMatchesDf(path):
    assert os.path.isfile(path), "file '%s' not found." % path
    df = pd.read_csv(filepath_or_buffer=path)
    df['id'] = df['#pointId'].astype(int)
    return df

def merge(exDf, maDf):
    def checkCols(df, cols):
        return pd.Series(cols).isin(df.columns).all()
    assert checkCols(exDf, ['id', 'class'])
    assert checkCols(maDf, ['id', 'label'])
    return pd.merge(exDf[['id', 'class']], maDf[['id', 'label']], on='id', how='left')

def confusionMatrix(exDf, maDf=None):
    merged = exDf
    if maDf is not None:
        merged = merge(exDf, maDf)
    assert merged.columns.all(['id', 'class', 'label'])
    cf = pd.crosstab(merged['class'], merged['label'],
                     rownames=['Classes (act)'], colnames=['Labels (pred)']).transpose()
    classes = cf.columns.values
    labels = cf.index
    offf = ['-'] + [c for c in classes if c in labels]
    cf['assigned'] = [l if l in offf else c for l, c in zip(labels, cf.idxmax(axis='columns'))]
    cf['hits'] = [0 if l == '-' else cf.at[i, l] for i, l in cf['assigned'].iteritems()]
    return (cf, classes, labels.to_list(), offf)

def printEval(exDf, maDf):
    print("examples ", exDf.shape)
    print("matches  ", maDf.shape)
    df = merge(exDf, maDf)
    cf, classes, labels, offf = confusionMatrix(df)
    print("Confusion Matrix")
    print(cf)

    totalExamples = exDf['id'].count()
    totalMatches = maDf.shape[0]
    tot = max(totalMatches, totalExamples)
    hits = cf['hits'].sum()
    misses = totalMatches - hits

    print('Classes          ', classes)
    print('Labels           ', labels, len(labels))
    print('Initial labels   ', offf)
    print('Total examples   %8d' % (totalExamples))
    print('Total matches    %8d' % (totalMatches))
    print('Hits             %8d (%10f%%)' % (hits, (hits/tot) * 100.0))
    print('Misses           %8d (%10f%%)' % (misses, (misses/tot) * 100.0))
    # print('Hits + Misses    %8d (%10f%%)' % (hits + misses, ((hits + misses)/tot) * 100.0))
    print('')
    return cf

def diffMinasMfog(examplesDf, minasDF, mfogDF):
    print("### Minas\n")
    printEval(examplesDf, minasDF)
    print("### Mfog\n")
    printEval(examplesDf, mfogDF)
    # [['id', 'og', 'label']]
    m = pd.merge(minasDF, mfogDF, on='id', how='left')
    diff = m[m['label_x'] != m['label_y']]
    toRename = {'clusterLabel_x': 'j_clL', 'clusterRadius_x': 'j_clR',
                'label_x': 'j_L', 'distance_x': 'j_D',
                'clusterLabel_y': 'c_clL', 'clusterRadius_y': 'c_clR',
                'label_y': 'c_L', 'distance_y': 'c_D'}
    toKeep = ['id', 'j_clL', 'j_clR', 'j_L', 'j_D', 'c_clL', 'c_clR', 'c_L', 'c_D']
    print("### Diff\n")
    print(diff.rename(columns=toRename)[toKeep])
    return diff

def getModelDf(path):
    assert os.path.isfile(path), "file '%s' not found." % path
    df = pd.read_csv(filepath_or_buffer=path)
    df['id'] = df['#id'].astype(int)
    msum = df['matches'].sum()
    df['matches'] = df['matches'].astype(float) / msum
    return df.drop(['#id', 'time'], axis=1)  # .sort_values('meanDistance')

def compareModelDf(a, b, path=None):
    if (path is not None):
        assert os.path.isdir(path), "dir '%s' not found." % path
    # figKArgs = {papertype: 'a4', }
    # + [ 'matches', 'meanDistance', 'radius']
    toDrop = ['id', 'label', 'category']
    aCl = a.drop(toDrop, axis=1)
    bCl = b.drop(toDrop, axis=1)
    c = aCl - bCl
    d = c[c != 0].abs().dropna(axis=1, how='all')
    #
    #     d = c[c != 0]
    #     d = d.abs().dropna(axis=1, how='all')
    #     dmin = d.min().min()
    #     g = plt.pcolor(d, norm=LogNorm(vmin=dmin, vmax=1), vmin=dmin, vmax=1)
    #     plt.colorbar()
    #     plt.show()
    fig, (ax0, ax1) = plt.subplots(2, 1)
    ax0.set_title('Full model')
    ax0.pcolor(aCl)
    ax1.pcolor(bCl)
    if (path is not None):
        fig.savefig(path + 'model-full.png', dpi=300)
    # 
    fig, (ax0, ax1) = plt.subplots(2, 1)
    ax0.set_title('Full diff')
    ax0.pcolor(c)
    ax1.pcolor(d)
    if (path is not None):
        fig.savefig(path + 'model-diff.png', dpi=300)
    # 
    m = pd.merge(a, b, on='id', how='left')
    diff = m[m['matches_x'] != m['matches_y']]
    diffIds = diff['id']
    e = a[a['id'].isin(diffIds)].drop(toDrop, axis=1)
    f = b[b['id'].isin(diffIds)].drop(toDrop, axis=1)
    g = e - f
    h = g[g != 0].abs().dropna(axis=1, how='all')
    fig, (ax0) = plt.subplots(1, 1)
    ax0.set_title('Merge ==id, !=matches')
    ax0.pcolor(h)
    # ax1.pcolor(f)
    if (path is not None):
        fig.savefig(path + 'model-diff-matches.png', dpi=300)
    return d


def main(
    examplesFN='datasets/test.csv',
    matchesFN='out/matches.csv',
    minasMatches='out/minas-og/2020-07-20T12-18-21.758/matches.csv',
    modelFN='datasets/model-clean.csv',
    mfogModelFN='out/model.csv',
    outputDir='out/',
):
    print(
        __file__ + '\n' +
        '\texamplesFN = %s\n' % examplesFN +
        '\tmatchesFN = %s\n' % matchesFN +
        '\tminasMatches = %s\n' % minasMatches +
        '\tmodelFN = %s\n' % modelFN +
        '\tmfogModelFN = %s\n' % mfogModelFN
    )
    examplesDf = getExamplesDf(examplesFN)
    countPerClass = examplesDf.groupby('class').count()[['id']]
    print("Count per class")
    print(countPerClass)

    mfogDF = getMatchesDf(matchesFN)
    minasDF = getMatchesDf(minasMatches)
    # ogdf = getOriginalMatchesDf('../../out/minas-og/2020-07-20T12-21-54.755/results')
    diffMinasMfog(examplesDf, minasDF, mfogDF)

    modelDF = getModelDf(modelFN)
    # minasFiModDF = getModelDf('../../out/minas-og/2020-07-22T01-19-11.984/model/653457_final.csv')
    mfogModelDF = getModelDf(mfogModelFN)
    compareModelDf(modelDF, mfogModelDF, outputDir)

    # d['meanDistance'].abs().sum()
    # d.min().min()
    # newIni = getModelDf('../../out/minas-og/2020-07-22T21-46-01.167/model/0_initial.csv')
    # newFin = getModelDf('../../out/minas-og/2020-07-22T21-46-01.167/model/653457_final.csv')
    # compareModelDf(modelDF, newIni)
    # compareModelDf(newIni, newFin)
    # compareModelDf(mfogModelDF, newIni)

if __name__ == "__main__":
    params = [
        'examplesFN',
        'matchesFN',
        'minasMatches',
        'modelFN',
        'mfogModelFN',
        'outputDir',
    ]
    main(**dict(zip(params, sys.argv[1:])))

