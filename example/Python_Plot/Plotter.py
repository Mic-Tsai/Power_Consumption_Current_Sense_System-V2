// ###################################################################################
// # Project: Current Sense System V2
// # Engineer:  Mic.Tsai
// # Date:  14 January 2020
// # Objective: Plot data into image
// # Usage: Python3
// ###################################################################################

// ###################################################################################
// # How to use it
// # python3 [Plotter.py] [your txt file] [Channel name copy paste into here]
// # Channel Name: ch1_iload ch1_volt ch1_pload ch1_vload
// ###################################################################################


import argparse, re, sys, os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
path = ''
flname = sys.argv[1]
try:
	chartType = sys.argv[2]
except:
	chartType = 'ch1_vload'
print('chartType:'+chartType)	

fl = flname.split('/')
for i in fl[:-1]:
	path = path+i+'/'

fw = open(flname, 'r')
rawdata = fw.read().strip()

ch1_list = []
ch2_list = []

ch1_vload = []
ch1_volt = []
ch1_iload = []
ch1_pload = []

ch2_vload = []
ch2_volt = []
ch2_iload = []
ch2_pload = []

vload_unit = 'V'
coltage_unit = 'mV'
iload_unit = 'mA'
pload_unit = 'mW'

line = rawdata.split('\n')
for aline in line:
	
	tmp = aline.split('||')
	ch1_list.append(tmp[0].lstrip())
	ch2_list.append(tmp[2].lstrip())

for item in ch1_list:
	tmp = item.split(' | ')
	for sub in tmp:
		if sub.count("V-load"):
			ch1_vload.append(float(re.search('\d+\.\d+', sub).group()))
		elif sub.count("Voltage"):
			ch1_volt.append(float(re.search('\d+\.\d+', sub).group()))
		elif sub.count("I-load"):
			ch1_iload.append(float(re.search('\d+\.\d+', sub).group()))
		elif sub.count("P-load"):
			ch1_pload.append(float(re.search('\d+\.\d+', sub).group()))

for item in ch2_list:
	tmp = item.split(' | ')
	for sub in tmp:
		if sub.count("V-load"):
			ch2_vload.append(float(re.search('\d+\.\d+', sub).group()))
		elif sub.count("Voltage"):
			ch2_volt.append(float(re.search('\d+\.\d+', sub).group()))
		elif sub.count("I-load"):
			ch2_iload.append(float(re.search('\d+\.\d+', sub).group()))
		elif sub.count("P-load"):
			ch2_pload.append(float(re.search('\d+\.\d+', sub).group()))

if chartType.lower().count('vload') or chartType.lower().count('v-load'):
	print('**vload')
	if chartType.lower().count('ch1'):
		y = ch1_vload
	else:
		y = ch2_vload
elif chartType.lower().count('volt'):
	print('**volt')
	if chartType.lower().count('ch1'):
		y = ch1_volt
	else:
		y = ch2_volt
elif chartType.lower().count('iload') or chartType.lower().count('i-load'):
	print('**iload')
	if chartType.lower().count('ch1'):
		y = ch1_iload
	else:
		y = ch2_iload
elif chartType.lower().count('pload') or chartType.lower().count('p-load'):
	print('**pload')
	if chartType.lower().count('ch1'):
		y = ch1_pload
	else:
		y = ch2_pload

x = np.linspace(1,len(y),len(y))
fig = plt.figure(1)
ax = plt.axes()
plt.xlim([0, len(y)])
plt.ylim([min(y)-(max(y)-min(y))/2,max(y)+(max(y)-min(y))/2])
plt.plot(x,y,ls='-',c='b')
plt.grid('on')
plt.title(chartType)
plt.savefig(path+chartType+'.png')
print("File Path:"+path+chartType+'.png')









