#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  linepoints.py
#  
#  Copyright 2019 Robson Araújo Lima <robson@robson-lenovo>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  
#
import csv
import numpy as np
import matplotlib.pyplot as plt


y1 = []
y1error = []

y2 = []
y2error = []

y3 = []
y3error = []

y4 = []
y4error = []

y5 = []
y5error = []

with open('Measurement Packets Received.csv') as csv_file:
	csv_reader = csv.reader(csv_file, delimiter=',')
	line_count = 0
	next(csv_reader)
	for row in csv_reader:
		if line_count == 3:
			break;
		y1.append(float(row[2]))
		y1error.append(float(row[3]))
		#elif line_count == 1:
		y2.append(float(row[5]))
		y2error.append(float(row[6]))
		#elif line_count == 2:
		y3.append(float(row[8]))
		y3error.append(float(row[9]))
		#elif line_count == 3:
		y4.append(float(row[11]))
		y4error.append(float(row[12]))
		#elif line_count == 4:
		y5.append(float(row[14]))
		y5error.append(float(row[15]))
		line_count += 1

bar_width = 0.25
ya = []
yb = []
yc = []
ya.append([y1[0],y2[0],y3[0],y4[0],y5[0]])
yb.append([y1[1],y2[1],y3[1],y4[1],y5[1]])
yc.append([y1[2],y2[2],y3[2],y4[2],y5[2]])

yaerror = []
yberror = []
ycerror = []
yaerror.append([y1error[0],y2error[0],y3error[0],y4error[0],y5error[0]])
yberror.append([y1error[1],y2error[1],y3error[1],y4error[1],y5error[1]])
ycerror.append([y1error[2],y2error[2],y3error[2],y4error[2],y5error[1]])

x = ['M. Pressão','Oxímetro','M. Glicose','Termômetro','ECG']
index = np.arange(len(x))

plt.title('Comparação da média de leituras entregues \n com sucesso em (%)', fontsize=12)

plt.bar(index, ya[0],  bar_width, label='Com Confirmação', yerr=yaerror[0], capsize=4)
plt.bar(index +  bar_width, yb[0],  bar_width, label='Sem Confirmação', yerr=yberror[0], capsize=4)
plt.bar(index +  bar_width*2, yc[0],  bar_width, label='Com Retransmissão', yerr=ycerror[0], capsize=4)

plt.xticks(index + (bar_width), x)
plt.grid(axis='y',alpha=0.9, linestyle=':')
#plt.yaxis.grid(True)

plt.legend()
plt.xlabel('')
plt.ylabel('Pacotes',fontsize=12)
plt.show()





