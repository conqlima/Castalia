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
        print(alteration)
        ax.bar(x_arrange + alteration[i], y_data_list[i], label = y_data_names[i], yerr = y_error[i], width = ind_width)
    ax.set_xticks(x_arrange - (ind_width/2))
    ax.set_xticklabels(x_data)
    ax.set_ylabel(y_label)
    ax.set_xlabel(x_label)
    ax.set_title(title)
    ax.legend(loc = 'lower right')

def readfile(y_data, y_error):
	with open('MeasurementsReceivedinManagerManagerInitiated.csv') as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		line_count = 0
		next(csv_reader)
		for row in csv_reader:
			j = 0
			temp_y_data = []
			temp_intervalo_confianca = []
			while j < (len(row)-3):
				if j == 0:
					j += 2
				else:
					j += 3
				temp_y_data.append(float(row[j]))
				temp_intervalo_confianca.append(float(row[j+1]))
			y_data.append(temp_y_data[:])
			y_error.append(temp_intervalo_confianca[:])
			line_count += 1

def main(args):
	x_data = ['M. Pressão','Oxímetro','M. Glicose','Termômetro','ECG']
	x_arrange = np.arange(len(x_data))
	y_data_names = ['noTimeMode','singleTime','timePeriodMode']
	y_data = []
	y_error = []
	readfile(y_data, y_error)
	groupedbarplot(x_arrange, x_data, y_data, y_data_names, y_error,'', 'delivered packets (%)', 'Measurements received by manager')
	plt.show()
	return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
