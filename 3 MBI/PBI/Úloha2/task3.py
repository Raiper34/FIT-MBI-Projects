"""
PBI Homework NO. 2 Task 3
@author: Filip Gulan
@mai: xgulan00@stud.fit.vutbr.cz
@date: 22.11.2017
"""

from pymol import cmd, CmdException
import math

def minmax(moleculeName):
    results = []
    model = cmd.get_model(moleculeName)
    for atom1 in model.atom:
        for atom2 in model.atom:
            if atom1.index is not atom2.index:
                results.append({'value': (((atom1.coord[0] - atom2.coord[0])**2) + ((atom1.coord[1] - atom2.coord[1])**2) + ((atom1.coord[2] - atom2.coord[2])**2)), 'atom1': atom1, 'atom2': atom2})
    maxPricedItem = max(results, key=lambda x: x['value'])
    minPricedItem = min(results, key=lambda x: x['value'])
    cmd.select("max", "index " + str(maxPricedItem['atom1'].index) + " or index " + str(maxPricedItem['atom2'].index))
    cmd.color("red", 'max')
    cmd.label("max", maxPricedItem['value'])
    cmd.select("min", "index " + str(minPricedItem['atom1'].index) + " or index " + str(minPricedItem['atom2'].index))
    cmd.color("blue", 'min')
    cmd.label("min", minPricedItem['value'])


cmd.extend('minmax', minmax)