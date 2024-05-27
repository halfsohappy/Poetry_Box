#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
[.py]
[Annabel Lee]
[]

I understand and have adhered to all the tenets of the Duke Community Standard
in creating this code.
Signed: [nml29]
"""

from poetpy import get_poetry
import pandas as pd
from pandas.io.json import json_normalize
import os
import shutil
import warnings
import random
warnings.simplefilter('ignore')

authors = get_poetry('author')

authors_df = pd.DataFrame.from_dict(authors)

poems = []
for author in authors['authors']:
    author_poems = get_poetry('author', author, 'author,title,lines,linecount')
    author_poems_df = json_normalize(author_poems)
    poems.append(author_poems_df)
    
poems_df = pd.concat(poems)


poems_df['lines'] = poems_df['lines'].apply(lambda x: ' \n '.join(x))

poems_df.to_csv("outputc.csv")  

poems_df["linecount"] = pd.to_numeric(poems_df["linecount"])

df_short = poems_df[poems_df['linecount'] <= 75]


print(df_short.info(memory_usage='deep'))

#%%
used_auth_list = []

path = '/Users/bugsy/Documents/poem2a/'

os.chdir(path)

for author in poems_df['author']:
    if author not in used_auth_list:
        used_auth_list.append(author)
        stra = str(author)
        os.mkdir(os.path.join(os.getcwd(),stra))


for number in range(len(poems_df)):
    stra = str(poems_df.iloc[number][1])
    os.chdir(os.path.join(path,stra))
    if poems_df.iloc[number][3] <= 150:
        mark = '_' if poems_df.iloc[number][3] <= 50 else '@' 
        with open(poems_df.iloc[number][0].replace('/', '\\')+'_'+str(poems_df.iloc[number][3])+ mark + '.txt', 'w') as f:
            f.write(poems_df.iloc[number][0])
            f.write('\n')
            f.write(poems_df.iloc[number][1])
            f.write('\n\n')
            dfst = poems_df.iloc[number][2]
            f.write(dfst)

#%%
ind_list = []

for number in range(len(poems_df)):
    stra = str(poems_df.iloc[number][1])
    os.chdir(os.path.join(path,stra))
    stra = '/'+stra
    cur_name = os.path.join(stra,poems_df.iloc[number][0].replace('/', '\\')+'_'+str(poems_df.iloc[number][3])+'.txt')
    if(poems_df.iloc[number][3] <= 75):
        ind_list.append(cur_name)

for ind in range(len(ind_list)):
    if('"' in ind_list[ind]):
        ind_list[ind] = ind_list[ind].replace('"','\\"')

with open('content_info.txt', 'w') as con:
    for pat in ind_list:
        con.write('"')
        con.write(pat)
        con.write('",\n')
#%%
high = 0
for poem in poems_df['author']:
    high = len(poem) if (len(poem)>high) else high

for poem in poems_df:
    print(poem)


# for subd in os.listdir(path):
#     whole_path = os.path.join(path,subd)
#     print(whole_path)
#     for files in os.listdir(os.path.join(path, subd)):
#         with open('content_info.txt', 'a') as con:
        
#%%
    
        
        
        
        