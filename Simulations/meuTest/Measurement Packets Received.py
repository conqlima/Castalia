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
	line_count = 1
	#pula as três primeiras linhas
	next(csv_reader)
	next(csv_reader)
	next(csv_reader)
	for row in csv_reader:
		y1.append(float(row[2]))
		y1error.append(float(row[3]))
		
		y2.append(float(row[5]))
		y2error.append(float(row[6]))
		
		y3.append(float(row[8]))
		y3error.append(float(row[9]))
		
		y4.append(float(row[11]))
		y4error.append(float(row[12]))
		
		y5.append(float(row[14]))
		y5error.append(float(row[15]))

		line_count += 1
		

index = [200,400,600,800,1000]
bar_width = 0.10
fig, axs = plt.subplots(3, 2)
axs[-1, -1].axis('off')

fig.suptitle('Média de leituras entregues com sucesso em (%) \n usando o modo de retransmissão', fontsize=14)

ax = axs[0,0]
ax.set_title('Medidor de pressão')
l1 = ax.errorbar(index, y1[0:5],  yerr=y1error[0:5], fmt='o--'	,  capsize=3,markersize=4)
l2 = ax.errorbar(index, y1[5:10], yerr=y1error[5:10], fmt='o--'	,  capsize=3,markersize=4)
l3 = ax.errorbar(index, y1[10:15],yerr=y1error[10:15], fmt='o--', capsize=3,markersize=4)
l4 = ax.errorbar(index, y1[15:20],yerr=y1error[15:20], fmt='o--', capsize=3,markersize=4)
l5 = ax.errorbar(index, y1[20:25],yerr=y1error[20:25], fmt='o--', capsize=3,markersize=4)
ax.grid(alpha=0.9, linestyle=':')
ax.set_xlabel('timeout em ms')     
#ax.set_ylabel('%')     

ax = axs[0,1]
ax.set_title('Oxímetro de pulso')
ax.errorbar(index, y2[0:5],  yerr=y2error[0:5],   fmt='o--',  capsize=3,markersize=4)
ax.errorbar(index, y2[5:10], yerr=y2error[5:10],  fmt='o--',  capsize=3,markersize=4)
ax.errorbar(index, y2[10:15],yerr=y2error[10:15], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y2[15:20],yerr=y2error[15:20], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y2[20:25],yerr=y2error[20:25], fmt='o--', capsize=3,markersize=4)
ax.grid(alpha=0.5, linestyle=':')
ax.set_xlabel('timeout em ms')       
#ax.set_ylabel('%') 


ax = axs[1,0]
ax.set_title('Medidor de glicose')
ax.errorbar(index, y3[0:5],  yerr=y3error[0:5], fmt='o--'  ,  capsize=3,markersize=4)
ax.errorbar(index, y3[5:10], yerr=y3error[5:10], fmt='o--' ,  capsize=3,markersize=4)
ax.errorbar(index, y3[10:15],yerr=y3error[10:15], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y3[15:20],yerr=y3error[15:20], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y3[20:25],yerr=y3error[20:25], fmt='o--', capsize=3,markersize=4)
ax.grid(alpha=0.5, linestyle=':')
ax.set_xlabel('timeout em ms')       
#ax.set_ylim([0,100])
#ax.set_ylabel('%') 

ax = axs[1,1]
ax.set_title('Termômetro')
ax.errorbar(index, y4[0:5],  yerr=y4error[0:5], fmt='o--'  ,  capsize=3,markersize=4)
ax.errorbar(index, y4[5:10], yerr=y4error[5:10], fmt='o--' ,  capsize=3,markersize=4)
ax.errorbar(index, y4[10:15],yerr=y4error[10:15], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y4[15:20],yerr=y4error[15:20], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y4[20:25],yerr=y4error[20:25], fmt='o--', capsize=3,markersize=4)
ax.grid(alpha=0.5, linestyle=':')
ax.set_xlabel('timeout em ms')      
#ax.set_ylabel('%') 

ax = axs[2,0]
ax.set_title('Eletrocardiograma')
ax.errorbar(index, y5[0:5],  yerr=y5error[0:5], fmt='o--'  ,  capsize=3,markersize=4)
ax.errorbar(index, y5[5:10], yerr=y5error[5:10], fmt='o--' ,  capsize=3,markersize=4)
ax.errorbar(index, y5[10:15],yerr=y5error[10:15], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y5[15:20],yerr=y5error[15:20], fmt='o--', capsize=3,markersize=4)
ax.errorbar(index, y5[20:25],yerr=y5error[20:25], fmt='o--', capsize=3,markersize=4)
ax.grid(alpha=0.5, linestyle=':')
ax.set_xlabel('timeout em ms')       
#ax.set_ylabel('%') 

# Labels to use in the legend for each line
line_labels = ['6', '7', '8', '9', '10']

# Create the legend
fig.legend((l1, l2, l3, l4, l5),     # The line objects
           (line_labels),   # The labels for each line
           loc='lower right',   # Position of legend
           title='Retransmissões'  # Title for the legend
           )

#espaçamento entre cada gráfico
plt.tight_layout()

#plt.ylim(0, 100)

#ajusta a posição do titulo
fig.subplots_adjust(top=0.85)

plt.show()





