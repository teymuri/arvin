import xml.etree.ElementTree as ET

class Num(ET.Element):
    def __init__(self, tok):
        self.tok = tok
        self.tag = "Num"
        self.attrib = {"type": type(self.val()),
                       "val": self.val()}
    def val(self):
        try:
            return int(self.tok)
        except ValueError:
            return float(self.tok)

def pylist(*args):
    L = ET.Element("list")
    for a in args:
        L.append(ET.Element(a))
    return L
print(list(pylist("1","s").iter()))
