import mysql.connector
from mysql.connector import (connection)

class MysqlConn():
    
    def __init__(self):
                   
        try:
            cnx = connection.MySQLConnection(user='root', 
                                             password='1234',
                                             host='127.0.0.1',
                                             database='faces_database')
            self.cnx = cnx
            self.cursor = cnx.cursor()
            self.err = None
            
        except mysql.connector.Error as err:
            self.cnx = None
            print err
            self.err = err
    
    def getAllFromTable(self, table):
        try: 
            query =  ("SELECT * from " + table)
            self.cursor.execute(query)
            rows = None        
            rows = self.cursor.fetchall()
            return rows
            
        except mysql.connector.Error as err:
            self.err = err
            print err
            return None
        
    def addNew(self, faceName, faceID):
        try:        
            query = ("INSERT INTO pessoa_dir (nome, dir_id) VALUES (%s, %s)")
            self.cursor.execute(query, (faceName, faceID))
            self.cnx.commit()
            if self.cursor.lastrowid:
                return self.cursor.lastrowid;
            else:
                return None;
            
        except mysql.connector.Error as err:
            self.err = err
            print err
            return None
    
    def getByDirID(self, id):  
        try:
            query = ("SELECT nome FROM pessoa_dir WHERE dir_id = %s")
            self.cursor.execute(query, (id,))
            rows = self.cursor.fetchall()
            rtnNome = None
            for row in rows:
                rtnNome = row[0]
             
            return rtnNome
            
        except mysql.connector.Error as err:
            self.err = err
            print err
            return None
        
    def __del__(self):
        self.cursor.close()
        self.cnx.close()
            
                                        
        