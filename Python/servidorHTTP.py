#!/usr/bin/env python

"""Simple HTTP Server With Upload.
This module builds on BaseHTTPServer by implementing the standard GET
and HEAD requests in a fairly straightforward manner.
"""

import os
import posixpath
import BaseHTTPServer
import urllib
import cgi
import shutil
import mimetypes
import json
from DllFace import DllFace
from MySQL import MysqlConn

mysqlConn = MysqlConn()
OPERATION_PREDICT = "1"
OPERATION_ADD_PHOTO = "2"
OPERATION_ADD_PHOTO_ID = "3"
OPERATION_TRAIN = "4"

try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO


class SimpleHTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):

    def do_GET(self):
        f = self.send_head()
        if f:
            self.copyfile(f, self.wfile)
            f.close()

    def do_HEAD(self):
        f = self.send_head()
        if f:
            f.close()

    def do_POST(self):

        print "Novo POST recebido..."        
        
        #Captura o tipo da operacao        
        operation = self.headers['Operation'] 
                
        #Recebe o arquivo enviado por POST
        if operation != OPERATION_TRAIN:
            form = cgi.FieldStorage(
                fp=self.rfile,
                headers=self.headers,
                environ={'REQUEST_METHOD':'POST',
                         'CONTENT_TYPE':self.headers['Content-Type'],})
            filename = form['myFile'].filename
            data = form['myFile'].file.read()
                
            #Salva o arquivo recebido no diretorio 'upload'
            path = self.translate_path(self.path) + "\\upload\\"
            fn = os.path.join(path, filename)    
            try:
                open(fn, "wb").write(data)
            except IOError:
                resp = { 'status'     : 'ERRO',
                         'message'    : 'Nao foi possivel criar o arquivo' }
                self.send_response(200,json.dumps(resp))
                return (False, "Nao foi possivel criar o arquivo.")
                
            print "Imagem gravada em: " + fn
            
        #Carrega a DLL para a deteccao de Faces
        dllFace = DllFace()
        dllResponse = None
        resp = None
        
        if operation == OPERATION_PREDICT:  
            print "Chamada DLL: dllFace.predict(" + fn + ")"
            dllResponse = dllFace.predict(fn) 
                        
        elif operation == OPERATION_ADD_PHOTO:
            print "Chamada DLL: dllFace.facesToPgm(''," + filename + ")"
            dllResponse = dllFace.facesToPgm("", filename) 
            
            try:
                jsonResponse = json.loads(dllResponse)
                mysqlRtn = mysqlConn.addNew(self.headers['Name'], jsonResponse['dirId'])
                
                if mysqlRtn == None:
                    resp = { 'status'     : 'ERRO',
                             'message'    : 'Erro ao adicionar pessoa:' + str(mysqlConn.err)}   
                    
            except ValueError:
                resp = { 'status'     : 'ERRO',
                         'message'    : 'String JSON incorreta:' + dllResponse}   
            
            
        elif operation == OPERATION_ADD_PHOTO_ID:
            dirID = self.headers['DirId']
            print "Chamada DLL: dllFace.facesToPgm(" + dirID + ", " + filename + ")"
            dllResponse = dllFace.facesToPgm(dirID, filename)                        
        
        elif operation == OPERATION_TRAIN:
            print "Chamada DLL: dllFace.train( )"
            dllResponse = dllFace.train()
        
        else:
            resp = { 'status'     : 'ERRO',
                     'message'    : 'Operacao para o componente DLL nao identificada!'}                           
            
        #Gera o retorno da requisicao conforme o retorno da DLL...
        if resp == None:
            try:
                jsonResponse = json.loads(dllResponse)
                print 'Retorno Dll: ' + dllResponse
                if jsonResponse['status'] == 'OK':
                  
                    if  operation == OPERATION_PREDICT:
                        name = mysqlConn.getByDirID(jsonResponse['dirId'])
    
                        if name == None:
                            name = jsonResponse['dirId']
                            
                        resp = {'status'     : jsonResponse["status"],
                                'message'    : jsonResponse["message"],
                                'name'       : name} 
                    
                    elif operation == OPERATION_ADD_PHOTO:
                        resp = {'status'     : jsonResponse["status"],
                                'message'    : jsonResponse["message"],
                                'dirId'      : jsonResponse["dirId"]} 
                    
                    else:
                         resp = {'status'     : jsonResponse["status"],
                                'message'    : jsonResponse["message"]}                       
                                                            
                else:
                    resp = {'status'     : jsonResponse["status"],
                            'message'    : jsonResponse["message"]}            
                            
            except ValueError:
                resp = { 'status'     : 'ERRO',
                         'message'    : 'String JSON incorreta:' + dllResponse}   
            
        print resp
        self.send_response(200,json.dumps(resp))   
    
    def send_head(self):
        path = self.translate_path(self.path)
        f = None
        if os.path.isdir(path):
            if not self.path.endswith('/'):
                # redirect browser - doing basically what apache does
                self.send_response(301)
                self.send_header("Location", self.path + "/")
                self.end_headers()
                return None
            for index in "index.html", "index.htm":
                index = os.path.join(path, index)
                if os.path.exists(index):
                    path = index
                    break
            else:
                return self.list_directory(path)
        ctype = self.guess_type(path)
        try:
            # Always read in binary mode. Opening files in text mode may cause
            # newline translations, making the actual size of the content
            # transmitted *less* than the content-length!
            f = open(path, 'rb')
        except IOError:
            self.send_error(404, "File not found")
            return None
        self.send_response(200)
        self.send_header("Content-type", ctype)
        fs = os.fstat(f.fileno())
        self.send_header("Content-Length", str(fs[6]))
        self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
        self.end_headers()
        return f
    
    def list_directory(self, path):
        """Helper to produce a directory listing (absent index.html).
        Return value is either a file object, or None (indicating an
        error).  In either case, the headers are sent, making the
        interface the same as for send_head().
        """
        try:
            list = os.listdir(path)
        except os.error:
            self.send_error(404, "No permission to list directory")
            return None
        list.sort(key=lambda a: a.lower())
        f = StringIO()
        f.write('<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">\n')
        f.write('<html xmlns="http://www.w3.org/1999/xhtml">\n')
        f.write("	<head>\n")
        f.write('		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />\n')
        f.write("\n")
        f.write("		<title>Reconhecimento de Faces - Mathias Schroeder</title>\n")
        f.write("\n")
        f.write('		<style type="text/css">\n')
        f.write("\n")
        f.write("body, html  { height: 100%;}\n")
        f.write("html, body, div, span, applet, object, iframe,\n")
        f.write("/*h1, h2, h3, h4, h5, h6,*/ p, blockquote, pre,\n")
        f.write("a, abbr, acronym, address, big, cite, code,\n")
        f.write("del, dfn, em, font, img, ins, kbd, q, s, samp,\n")
        f.write("small, strike, strong, sub, sup, tt, var,\n")
        f.write("b, u, i, center,\n")
        f.write("dl, dt, dd, ol, ul, li,\n")
        f.write("fieldset, form, label, legend,\n")
        f.write("table, caption, tbody, tfoot, thead, tr, th, td {\n")
        f.write("	margin: 0;\n")
        f.write("	padding: 0;\n")
        f.write("	border: 0;\n")
        f.write("	outline: 0;\n")
        f.write("	font-size: 100%;\n")
        f.write("	vertical-align: baseline;\n")
        f.write("}\n")
        f.write("body { line-height: 1; }\n")
        f.write("ol, ul { list-style: none; }\n")
        f.write("blockquote, q { quotes: none; }\n")
        f.write("blockquote:before, blockquote:after, q:before, q:after { content: ''; content: none; }\n")
        f.write(":focus { outline: 0; }\n")
        f.write("del { text-decoration: line-through; }\n")
        f.write("table {border-spacing: 0; } /* IMPORTANT, I REMOVED border-collapse: collapse; FROM THIS LINE IN ORDER TO MAKE THE OUTER BORDER RADIUS WORK */\n")
        f.write("\n")
        f.write("/*------------------------------------------------------------------ */\n")
        f.write("\n")
        f.write("/*This is not important*/\n")
        f.write("body{\n")
        f.write("	font-family:Arial, Helvetica, sans-serif;\n")
        f.write("	margin:0 auto;\n")
        f.write("	width:520px;\n")
        f.write("}\n")
        f.write("a:link {\n")
        f.write("	color: #666;\n")
        f.write("	font-weight: bold;\n")
        f.write("	text-decoration:none;\n")
        f.write("}\n")
        f.write("a:visited {\n")
        f.write("	color: #666;\n")
        f.write("	font-weight:bold;\n")
        f.write("	text-decoration:none;\n")
        f.write("}\n")
        f.write("a:active,\n")
        f.write("a:hover {\n")
        f.write("	color: #bd5a35;\n")
        f.write("	text-decoration:underline;\n")
        f.write("}\n")
        f.write("\n")
        f.write("\n")
        f.write("/*\n")
        f.write("Table Style - This is what you want\n")
        f.write("------------------------------------------------------------------ */\n")
        f.write("table a:link {\n")
        f.write("	color: #666;\n")
        f.write("	font-weight: bold;\n")
        f.write("	text-decoration:none;\n")
        f.write("}\n")
        f.write("table a:visited {\n")
        f.write("	color: #999999;\n")
        f.write("	font-weight:bold;\n")
        f.write("	text-decoration:none;\n")
        f.write("}\n")
        f.write("table a:active,\n")
        f.write("table a:hover {\n")
        f.write("	color: #bd5a35;\n")
        f.write("	text-decoration:underline;\n")
        f.write("}\n")
        f.write("table {\n")
        f.write("	font-family:Arial, Helvetica, sans-serif;\n")
        f.write("	color:#666;\n")
        f.write("	font-size:12px;\n")
        f.write("	text-shadow: 1px 1px 0px #fff;\n")
        f.write("	background:#eaebec;\n")
        f.write("	margin:20px;\n")
        f.write("	border:#ccc 1px solid;\n")
        f.write("\n")
        f.write("	-moz-border-radius:3px;\n")
        f.write("	-webkit-border-radius:3px;\n")
        f.write("	border-radius:3px;\n")
        f.write("\n")
        f.write("	-moz-box-shadow: 0 1px 2px #d1d1d1;\n")
        f.write("	-webkit-box-shadow: 0 1px 2px #d1d1d1;\n")
        f.write("	box-shadow: 0 1px 2px #d1d1d1;\n")
        f.write("}\n")
        f.write("table th {\n")
        f.write("	padding:21px 25px 22px 25px;\n")
        f.write("	border-top:1px solid #fafafa;\n")
        f.write("	border-bottom:1px solid #e0e0e0;\n")
        f.write("\n")
        f.write("	background: #ededed;\n")
        f.write("	background: -webkit-gradient(linear, left top, left bottom, from(#ededed), to(#ebebeb));\n")
        f.write("	background: -moz-linear-gradient(top,  #ededed,  #ebebeb);\n")
        f.write("}\n")
        f.write("table th:first-child{\n")
        f.write("	text-align: left;\n")
        f.write("	padding-left:20px;\n")
        f.write("}\n")
        f.write("table tr:first-child th:first-child{\n")
        f.write("	-moz-border-radius-topleft:3px;\n")
        f.write("	-webkit-border-top-left-radius:3px;\n")
        f.write("	border-top-left-radius:3px;\n")
        f.write("}\n")
        f.write("table tr:first-child th:last-child{\n")
        f.write("	-moz-border-radius-topright:3px;\n")
        f.write("	-webkit-border-top-right-radius:3px;\n")
        f.write("	border-top-right-radius:3px;\n")
        f.write("}\n")
        f.write("table tr{\n")
        f.write("	text-align: center;\n")
        f.write("	padding-left:20px;\n")
        f.write("}\n")
        f.write("table tr td:first-child{\n")
        f.write("	text-align: left;\n")
        f.write("	padding-left:20px;\n")
        f.write("	border-left: 0;\n")
        f.write("}\n")
        f.write("table tr td {\n")
        f.write("	padding:18px;\n")
        f.write("	border-top: 1px solid #ffffff;\n")
        f.write("	border-bottom:1px solid #e0e0e0;\n")
        f.write("	border-left: 1px solid #e0e0e0;\n")
        f.write("\n")
        f.write("	background: #fafafa;\n")
        f.write("	background: -webkit-gradient(linear, left top, left bottom, from(#fbfbfb), to(#fafafa));\n")
        f.write("	background: -moz-linear-gradient(top,  #fbfbfb,  #fafafa);\n")
        f.write("}\n")
        f.write("table tr.even td{\n")
        f.write("	background: #f6f6f6;\n")
        f.write("	background: -webkit-gradient(linear, left top, left bottom, from(#f8f8f8), to(#f6f6f6));\n")
        f.write("	background: -moz-linear-gradient(top,  #f8f8f8,  #f6f6f6);\n")
        f.write("}\n")
        f.write("table tr:last-child td{\n")
        f.write("	border-bottom:0;\n")
        f.write("}\n")
        f.write("table tr:last-child td:first-child{\n")
        f.write("	-moz-border-radius-bottomleft:3px;\n")
        f.write("	-webkit-border-bottom-left-radius:3px;\n")
        f.write("	border-bottom-left-radius:3px;\n")
        f.write("}\n")
        f.write("table tr:last-child td:last-child{\n")
        f.write("	-moz-border-radius-bottomright:3px;\n")
        f.write("	-webkit-border-bottom-right-radius:3px;\n")
        f.write("	border-bottom-right-radius:3px;\n")
        f.write("}\n")
        f.write("table tr:hover td{\n")
        f.write("	background: #f2f2f2;\n")
        f.write("	background: -webkit-gradient(linear, left top, left bottom, from(#f2f2f2), to(#f0f0f0));\n")
        f.write("	background: -moz-linear-gradient(top,  #f2f2f2,  #f0f0f0);	\n")
        f.write("}\n")
        f.write("\n")
        f.write("	</style>\n")
        f.write("\n")
        f.write("</head>\n")
        f.write("\n")
        f.write("	<body>\n")
        f.write("<br><br>")
        f.write("		<table cellspacing='0'>\n")
        f.write("			<tr> <th style='text-align:center' colspan='3'> Lista de usuarios cadastrados  </th> </tr>\n")
        f.write("			<tr>\n")
        f.write("				<th>ID</th>\n")
        f.write("				<th>Nome</th>\n")
        f.write("				<th>ID do Diretorio</th>\n")
        f.write("			</tr>\n")
        f.write("			<!-- Table Header -->\n")

        rows = mysqlConn.getAllFromTable('pessoa_dir')
        if rows:
            for row in rows:
                f.write("			<tr>\n")
                f.write("				<td>" + str(row[0]) + "</td>\n")
                f.write("				<td>" + str(row[1]) + "</td>\n")
                f.write("				<td>" + str(row[2]) + "</td>\n")
                f.write("			</tr>\n")

        f.write("\n")
        f.write("		</table>\n")
        f.write("	</body>\n")
        f.write("</html>\n")
        
        length = f.tell()
        f.seek(0)
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.send_header("Content-Length", str(length))
        self.end_headers()
        return f

    def translate_path(self, path):
        """Translate a /-separated PATH to the local filename syntax.
        Components that mean special things to the local file system
        (e.g. drive or directory names) are ignored.  (XXX They should
        probably be diagnosed.)
        """
        # abandon query parameters
        path = path.split('?',1)[0]
        path = path.split('#',1)[0]
        path = posixpath.normpath(urllib.unquote(path))
        words = path.split('/')
        words = filter(None, words)
        path = os.getcwd()
        for word in words:
            drive, word = os.path.splitdrive(word)
            head, word = os.path.split(word)
            if word in (os.curdir, os.pardir): continue
            path = os.path.join(path, word)
        return path

    def copyfile(self, source, outputfile):
        """Copy all data between two file objects.
        The SOURCE argument is a file object open for reading
        (or anything with a read() method) and the DESTINATION
        argument is a file object open for writing (or
        anything with a write() method).
        The only reason for overriding this would be to change
        the block size or perhaps to replace newlines by CRLF
        -- note however that this the default server uses this
        to copy binary data as well.
        """
        shutil.copyfileobj(source, outputfile)

    def guess_type(self, path):
        """Guess the type of a file.
        Argument is a PATH (a filename).
        Return value is a string of the form type/subtype,
        usable for a MIME Content-type header.
        The default implementation looks the file's extension
        up in the table self.extensions_map, using application/octet-stream
        as a default; however it would be permissible (if
        slow) to look inside the data to make a better guess.
        """

        base, ext = posixpath.splitext(path)
        if ext in self.extensions_map:
            return self.extensions_map[ext]
        ext = ext.lower()
        if ext in self.extensions_map:
            return self.extensions_map[ext]
        else:
            return self.extensions_map['']

    if not mimetypes.inited:
        mimetypes.init() # try to read system mime.types
    extensions_map = mimetypes.types_map.copy()
    extensions_map.update({
        '': 'application/octet-stream', # Default
        '.py': 'text/plain',
        '.c': 'text/plain',
        '.h': 'text/plain',
        })


def test(HandlerClass = SimpleHTTPRequestHandler,
         ServerClass = BaseHTTPServer.HTTPServer):
    BaseHTTPServer.test(HandlerClass, ServerClass)

if __name__ == '__main__':
    test()