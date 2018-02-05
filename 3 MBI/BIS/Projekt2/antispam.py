#!/usr/bin/env python3.6
# -*- coding: utf-8 -*-
# BIS Project 2 2017
# Author: Filip Gulan (xgulan00)
# Mail: xgulan00@stud.fit.vutbr.cz
# Date: 15.12.2017
# Name: antispam.py

import sys
import re
import eml_parser

"""
Parse opened mail and get his subject
Input: filename or path to mail
Output: None, if problem with mail file, otherwise dictionary with subject_raw and subject splited into array
"""
def getMail(filename):
    try:
        with open(filename, 'rb') as fhdl:
            mail = {}
            raw_email = fhdl.read()
            parsed_eml = eml_parser.eml_parser.decode_email_b(raw_email, include_raw_body=True)
            mail['subject_raw'] = parsed_eml['header']['subject']
            mail['subject'] = re.sub("\n", " ", mail['subject_raw'])
            mail['subject'] = re.sub("(\.|\,|\?|\!|\:|\;|\'\")", "", mail['subject']).lower().split()
            return mail
    except:
        return None

"""
Detect if it is spam or not by mail subject
Input: dictionary that contains subject and subject_raw and blackWords array
Output: detected spam word or None 
"""
def isSpam(mail, blackWords):
    for word in blackWords:
        if " " in word: #it is phrase
            if word in mail['subject_raw']:
                return word
        else: #it is single  word
            if word in mail['subject']:
                return word
    return None

"""
Get blackwords from file and store into array
Output: array of loaded blackwords
"""
def getBlackWords():
    words = []
    with open('black_words.txt', 'r+') as fhdl:
        for line in fhdl.readlines():
            word = re.sub("\n", "", line)
            if word == '' or '#' in word:
                continue
            words.append(word.lower())
    return words

"""
Evaluate given mail
Input: filename or path of mail and blackwords array
Output: result of evaluation as string
"""
def evaluate(filename, blackWords):
    mail = getMail(filename)
    if mail:
        result = isSpam(mail, blackWords)
        if result:
            return "SPAM - subject contains blackword \"" + result + "\""
        else:
            return "OK"
    else:
        return "FAIL - failed to open or parse email file"

"""
Main
"""
blackWords = getBlackWords()
argC = 1
while argC < len(sys.argv):
    print(sys.argv[argC] + " - " + evaluate(sys.argv[argC], blackWords))
    argC += 1
