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

with open('Number of transmissions retries per packet.csv') as csv_file:
	csv_reader = csv.reader(csv_file, delimiter=',')
	line_count = 0
	next(csv_reader)
	for row in csv_reader:
		if line_count == 0:
			y1.append([float(i) for i in row[1:8]])
			y1error.append([float(i) for i in row[8:15]])
		elif line_count == 1:
			y2.append([float(i) for i in row[1:8]])
			y2error.append([float(i) for i in row[8:15]])
		elif line_count == 2:
			y3.append([float(i) for i in row[1:8]])
			y3error.append([float(i) for i in row[8:15]])
		elif line_count == 3:
			y4.append([float(i) for i in row[1:8]])
			y4error.append([float(i) for i in row[8:15]])
		elif line_count == 4:
			y5.append([float(i) for i in row[1:8]])
			y5error.append([float(i) for i in row[8:15]])
		line_count += 1

index = [1,2,3,4,5,6,7]

plt.suptitle('Média de retransmissão de pacotes', fontsize=16)

l1 = plt.errorbar(index, y1[0],  	yerr=y1error[0], fmt='o--')
l2 = plt.errorbar(index, y2[0], 	yerr=y2error[0], fmt='o--')
l3 = plt.errorbar(index, y3[0],		yerr=y3error[0], fmt='o--')
l4 = plt.errorbar(index, y4[0],		yerr=y4error[0], fmt='o--')
l5 = plt.errorbar(index, y5[0],		yerr=y5error[0], fmt='o--')
plt.grid(alpha=0.9, linestyle=':')         

# Labels to use in the legend for each line
line_labels = ['200', '400', '600', '800', '1000']

# Create the legend
plt.legend((l1, l2, l3, l4, l5),     # The line objects
           (line_labels),   # The labels for each line
           loc='upper right',   # Position of legend
           title='timeouts'  # Title for the legend
           )

plt.xlabel('Retransmissões')
plt.ylabel('pacotes')
plt.show()





