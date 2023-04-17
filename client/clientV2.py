#!/usr/bin/env python3

import socket
import threading
import time

ipAddr = "127.0.0.1"  # localhost
port = 5000


class Client:
    def __init__(self):
        self.username = None
        self.socket = None

    def connect_to_server(self):
        # Connect to the server
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((ipAddr, port))

        # Prompt the user to enter a username
        self.username = input("Enter a username to join the server: ")
        while not self.username:
            self.username = input(
                "Username cannot be empty. Please try again: ")

        # Send the username to the server to join the group
        self.socket.sendall(self.username.encode())

        # Start a separate thread to receive messages from the server
        receive_thread = threading.Thread(target=self.receive)
        receive_thread.start()

    def receive(self):
        while True:
            try:
                # Receive Message From Server
                message = self.socket.recv(1024).decode()
                print(message)
            except:
                # Close Connection When Error
                print("An error occured!")
                self.socket.close()
                break

    def handle_connection(self):
        while True:
            print(" 'quit' - disconnect from server")
            print(" 'join' - join the public group")
            print(" 'exit' - leave the public group ")
            print(" 'usrs' - Get users in the public group ")
            print(" 'post' - Post a message in the public group ")
            print(" 'mesg' - Get a message from the public group ")
            print(" 'grps' - Get the list of groups available to join ")
            print(" 'join <group_id>' - join a private group ")
            print(" 'exit <group_id>' - exit a group ")
            print(" 'usrs <group_id>' - Get the users in a group")
            print(" 'post <group_id>' - Post a message to a group")
            print(" 'get <group_id>' - Get a message from a group")
            message = input("Enter a command: ")

            if message == 'quit':
                self.socket.send(b"%quit\n")
                self.socket.close()
                print("You have left the group.")
                time.sleep(1)
                exit()
            elif message == 'exit':
                self.socket.send(b"%exit\n")
            # Check for join
            elif message[:4] == 'join':
                # Check if group is included
                if message[5:]:
                    # Check for valid number
                    group_id = int(message[5:])
                    if group_id < 0 or group_id > 5:
                        print("Invalid group!")
                        continue

                # If all was successful, send command
                self.socket.send(b"%" + message.encode() + b"\n")
            elif message == 'usrs':
                 # Check if group is included
                if message[5:]:
                    # Check for valid number
                    group_id = int(message[5:])
                    if group_id < 0 or group_id > 5:
                        print("Invalid group!")
                        continue
                self.socket.send(b"%" + message.encode() + b"\n")
            elif message == 'post':
                self.socket.send(b"%post\n")
                subject = input("What's the message subject? ")
                self.socket.send(subject.encode("utf-8") + b"\n")
                contents = input("What's the message content? ")
                self.socket.send(contents.encode("utf-8") + b"\n")
            elif message == 'mesg':
                self.socket.send(b"%mesg\n")
                mesg_id = input("What's the message id? ")
                self.socket.send(mesg_id.encode("utf-8") + b"\n")
            elif message == 'grps':
                self.socket.send(b"%grps\n")
            else:
                print("Invalid command. Please try again.")


# Main function to run the client program
def main():
    client = Client()
    client.connect_to_server()
    client.handle_connection()


if __name__ == '__main__':
    main()
