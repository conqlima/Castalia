#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  MeasurementsReceivedinManagerManagerInitiated.py
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
plt.rcParams.update({'font.size': 12})

def stackedbarplot(x_arrange, x_data, y_data_list, y_data_names, y_error, x_label="", y_label="", title=""):
    print(y_data_list)
    y_data_sub_list = []
    _, ax = plt.subplots()
    # Draw bars, one category at a time
    for i in range(0, len(y_data_list)):
        if i == 0:
            print(y_data_list[i])
            ax.bar(x_arrange, y_data_list[i], align = 'center', label = y_data_names[i], yerr = y_error[i])
        elif i == 1:
            print(y_data_list[i])
            ax.bar(x_arrange, y_data_list[i], bottom = y_data_list[i-1], align = 'center', label = y_data_names[i], yerr = y_error[i])
        else:
            print(y_data_list[i])
            # For each category after the first, the bottom of the
            # bar will be the top of the last category
            y_data_sub_list = y_data_list[:i]
            print(y_data_sub_list)
            print([sum(x) for x in zip(*y_data_sub_list)])
            ax.bar(x_arrange, y_data_list[i], bottom = [sum(x) for x in zip(*y_data_sub_list)], align = 'center', label = y_data_names[i], yerr = y_error[i])
    ax.set_xticks(x_arrange)
    ax.set_xticklabels(x_data)
    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.set_title(title)
    ax.legend(loc = 'lower right')

def groupedbarplot(x_arrange, x_data, y_data_list, y_data_names, y_error, x_label="", y_label="", title=""):
    _, ax = plt.subplots()
    # Total width for all bars at one x location
    total_width = 0.8
    # Width of each individual bar
    ind_width = total_width / len(y_data_list)
    # This centers each cluster of bars about the x tick mark
    alteration = np.arange(-(total_width/2), total_width/2, ind_width)

    # Draw bars, one category at a time
    for i in range(0, len(y_data_list)):
        # Move the bar to the right on the x-axis so it doesn't
        # overlap with previously drawn ones
        #print(alteration)
        ax.bar(x_arrange + alteration[i], y_data_list[i], label = y_data_names[i], yerr = y_error[i], width = ind_width, capsize=3)
    ax.set_xticks(x_arrange - (ind_width/2))
    ax.set_xticklabels(x_data)
    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.set_title(title)
    ax.grid(axis='y',alpha=0.9, linestyle=':')
    ax.legend(loc = 'lower left')

def readfile(y_data, y_error):
	with open('MeasurementsReceivedinManagerAgentInitiated.csv') as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		line_count = 0
		next(csv_reader)
		for row in csv_reader:
			j = 0
			temp_y_data = []
			temp_intervalo_confianca = []
			while j < (len(row)-4):
				if j == 0:
					j += 2
				else:
					j += 4
				#print(j)
				temp_y_data.append(float(row[j]))
				temp_intervalo_confianca.append(float(row[j+2]))
			y_data.append(temp_y_data[:])
			y_error.append(temp_intervalo_confianca[:])
			line_count += 1
#English
# def main(args):
	# x_data = ['Blood Plessure\nMonitor','Oximeter','Glucose\nmeter','Thermometer','ECG']
	# x_arrange = np.arange(len(x_data))
	# y_data_names = ['confirmedMode','noConfirmedMode','retransmissionMode']
	# y_data = []
	# y_error = []
	# readfile(y_data, y_error)
	# groupedbarplot(x_arrange, x_data, y_data, y_data_names, y_error,'', 'delivered measurements (%)', '')
	# #stackedbarplot(x_arrange, x_data, y_data, y_data_names, y_error,'', 'delivered measurements (%)', '')
	# plt.show()
	# return 0
	
#Português
def main(args):
	x_data = ['Medidor de\nPressão Arterial','Oxímetro','Medidor\nde Glicose','Termômetro','ECG']
	x_arrange = np.arange(len(x_data))
	y_data_names = ['Com Confirmação','Sem Confirmação','Retransmissão']
	y_data = []
	y_error = []
	readfile(y_data, y_error)
	groupedbarplot(x_arrange, x_data, y_data, y_data_names, y_error,'', 'leituras entregues (%)', '')
	#stackedbarplot(x_arrange, x_data, y_data, y_data_names, y_error,'', 'delivered measurements (%)', '')
	plt.show()
	return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
