echo "75.123.253.22	master" >> /etc/hosts
echo "75.123.253.56 	slave1" >> /etc/hosts
echo "75.123.253.34 	slave2" >> /etc/hosts
echo "75.123.253.54	slave3" >> /etc/hosts
echo "75.123.253.60 	slave4" >> /etc/hosts
echo "75.123.253.46	slave5" >> /etc/hosts

sudo adduser abena
sudo apt install openssh-server

su - abena
ssh-keygen -t dsa
ssh-copy-id slave1
ssh-copy-id slave2
ssh-copy-id slave3
ssh-copy-id slave4
ssh-copy-id slave5

eval `ssh-agent`
ssh-add ~/.ssh/id_dsa

sudo apt-get install nfs-kernel-server
mkdir /home/abena/cloud
echo "/home/abena/cloud *(rw,sync,no_root_squash,no_subtree_check)" >> /etc/exports

exportfs -a
sudo systemctl nfs-kernel-server restart

sudo apt-get install nfs-common
mkdir /home/abena/cloud
sudo mount -t nfs master:/home/abena/cloud ~/cloud

echo "master:/home/abena/cloud /home/abena/cloud nfs" >> /etc/fstab
