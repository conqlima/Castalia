#!/usr/bin/env python
#-*- coding: utf-8 -*-
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

with open('Application level latency in ms.csv') as csv_file:
	csv_reader = csv.reader(csv_file, delimiter=',')
	line_count = 0
	next(csv_reader)
	for row in csv_reader:
		if line_count == 1:
			break;
		y1.append([float(i) for i in row[1:32]])
		line_count += 1

index = ['[0,30)','[30,60)','[60,90)','[90,120)','[120,150)','[150,180)','[180,210)',' [210,240) ',' [240,270) ',' [270,300) ',' [300,330) ',' [330,360) ',' [360,390) ',' [390,420) ',' [420,450) ',' [450,480) ',' [480,510) ',' [510,540) ',' [540,570) ',' [570,600) ',' [600,630) ',' [630,660) ',' [660,690) ',' [690,720) ',' [720,750) ',' [750,780) ',' [780,810) ',' [810,840) ',' [840,870) ',' [870,900) ',' [900,inf) ']
x = np.arange(len(index))

plt.bar(x, y1[0])
plt.xticks(x, index, rotation=90)  

plt.tight_layout()
plt.xlabel('milissegundos')
plt.ylabel("quantidade de pacotes")
plt.title('Latência da camada de aplicação em milissegundos')
plt.grid(linestyle='dotted')
plt.show()





