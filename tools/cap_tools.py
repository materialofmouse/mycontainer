#!/usr/bin/python3
import json 
import sys

import capability


class cap_tools:
    def __init__(self, file_path: str):
        self.file_path = file_path
        self.file_list = []
        self.capp = capability.capability()
        self.headers = {
                'Inh': {'start': -1, 'end': -1},
                'Eff': {'start': -1, 'end': -1},
                'Prm': {'start': -1, 'end': -1},
        }

    def read_from_conf(self) -> dict:
        file = open(self.file_path, 'r')
        cap_type = ""
        cap_dict = {}
        line_count = 0
        self.file_list = []
        for tmp in file.readlines():
            self.file_list.append(tmp)
            text = tmp.replace("\n", "")
            if text == "":
                self.headers[cap_type]['end'] = line_count
            elif "[" in text:
                cap_type = text[1:-1]
                self.headers[cap_type]['start'] = line_count
                cap_dict[cap_type] = []
            elif text is not None and text != "":
                cap_name = self.capp.cap_name_from_value(int(text))
                if cap_name is not None:
                    cap_dict[cap_type].append(cap_name)
                else:
                    pass
            line_count += 1

        file.close()
        return cap_dict

    def insert_caps(self, cap: str, cap_type: str) -> bool:
        cap_number = self.capp.CAPS[cap]
        isFound = False
        index = self.headers[cap_type]['start']
        while not isFound:
            index += 1
            if index == self.headers[cap_type]['end']:
                break

            cap_current = int(self.file_list[index])
            if cap_number == cap_current:
                #print('already cap set')
                break
            if cap_number < cap_current:
                #print('insert cap! cap_n:{} cap_c:{} index:{}'.format(cap_number, cap_current, index))
                isFound = True
                self.file_list.insert(index, str(cap_number)+'\n')

            if index+1 < self.headers[cap_type]['end']:
                cap_next = int(self.file_list[index+1])
                if cap_next < cap_number:
                    continue
                if cap_current < cap_number and cap_number > cap_next:
                    isFound = True
                    self.file_list.insert(index, str(cap_number)+'\n')
            else:
                isFound = True
                self.file_list.insert(index+1, str(cap_number)+'\n')

        return isFound

    def drop_caps(self, cap: str, cap_type: str) -> bool:
        cap_number = self.capp.CAPS[cap]
        isFound = False
        index = self.headers[cap_type]['start']
        while not isFound:
            index += 1
            if index == self.headers[cap_type]['end']:
                break

            cap_current = int(self.file_list[index])
            if cap_current == cap_number:
                __cap = self.file_list.pop(index)
                #print("cap drop index:{} caps:{}".format(index, __cap))
                isFound = True

        return isFound

    def write_cap(self):
        file = open(self.file_path, 'w')
        file.writelines(self.file_list)
        file.close()

    def add_cap(self, caps: list[str], cap_type: str):
        for cap in caps:
            isWrite = self.insert_caps(cap, cap_type)
            if isWrite:
                self.write_cap()
                self.read_from_conf()

    def drop_cap(self, caps: list[str], cap_type: str):
        for cap in caps:
            isWrite = self.drop_caps(cap, cap_type)
            if isWrite:
                self.write_cap()
                self.read_from_conf()


if __name__ == '__main__':
    option = sys.argv
    tool = cap_tools('./config/capabilities.conf')
    cap_dict = tool.read_from_conf()
    
    if option[1] is None or option[1] == "":
        print(json.dumps(cap_dict))
    elif option[1] == 'all':
        tool.add_cap(tool.capp.CAPS.keys(), "Inh")
        tool.add_cap(tool.capp.CAPS.keys(), "Prm")
        tool.add_cap(tool.capp.CAPS.keys(), "Eff")
    elif option[1][0:3] == 'CAP':
        tool.drop_cap([option[1]], option[2])

    print(json.dumps(tool.read_from_conf()))
