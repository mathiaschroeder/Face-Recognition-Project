# -*- coding: utf-8 -*-
"""
Created on Tue Sep 08 01:57:36 2015

@author: Mathias Schroeder
"""

from ctypes import *

class DllFace():
        
    def __init__(self):
        self.dllFace = cdll.LoadLibrary('FacesDLL.dll')        
        
    def train(self):
        self.dllFace.train.restype = c_char_p
        dllResponse = c_char_p(self.dllFace.train())
        return dllResponse.value
    
    def facesToPgm(self, dirId, imageName):
        self.dllFace.facesToPgm.restype = c_char_p
        dllResponse = c_char_p(self.dllFace.facesToPgm(dirId, imageName))
        return dllResponse.value
        
    def predict(self,path):
        self.dllFace.predict.restype = c_char_p
        dllResponse = c_char_p(self.dllFace.predict(path))
        return dllResponse.value
        