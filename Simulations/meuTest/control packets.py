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

with open('control packets.csv') as csv_file:
	csv_reader = csv.reader(csv_file, delimiter=',')
	#line_count = 0
	next(csv_reader)
	for row in csv_reader:
		#if line_count == 0:
			y1.append(float(row[3]))
			y1error.append(float(row[4]))

index = [200,400,600,800,1000]
#index = np.arange(len(x))

plt.suptitle('Média de pacotes de controles enviados e recebidos', fontsize=16)

l1 = plt.errorbar(index, y1[0:5],  	yerr=y1error[0:5], fmt='o--')
l2 = plt.errorbar(index, y1[5:10], 		yerr=y1error[5:10], fmt='o--')
l3 = plt.errorbar(index, y1[10:15],		yerr=y1error[10:15], fmt='o--')
l4 = plt.errorbar(index, y1[15:20],		yerr=y1error[15:20], fmt='o--')
l5 = plt.errorbar(index, y1[20:25],		yerr=y1error[20:25], fmt='o--')
plt.grid(alpha=0.9, linestyle=':')         

# Labels to use in the legend for each line
line_labels = [6, 7, 8, 9, 10]

# Create the legend
plt.legend((l1, l2, l3, l4, l5),     # The line objects
           (line_labels),   # The labels for each line
           loc='upper right',   # Position of legend
           title='Retransmissões'  # Title for the legend
           )

plt.xlabel('timeouts em ms')
plt.ylabel('Quantidade de pacotes')
plt.show()





