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


y = [8.45, 0.15, 1.24]
yerror = [0.15, 0.02, 0.12146]
x = ['Com confirmação', 'Sem confirmação', 'Com retransmissão']
#organiza as label para serem plotadas no eixo x
index = np.arange(len(x))

plt.bar(index, y, yerr=yerror)
plt.xticks(index, x)

plt.ylabel('Associações')
#plt.xlabel('Modos de operação')
plt.title('Média de associações por modo de operação')
plt.grid(linestyle='dotted')

plt.show()





