# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************
#
# Initialize SQL tables.
#
DROP TABLE IF EXISTS books;
CREATE TABLE books
(
	id		INT UNSIGNED AUTO_INCREMENT NOT NULL,
	PRIMARY KEY	(id),
	isbn		CHAR(10),
	title		VARCHAR(255),
	renter_id	INT
) ENGINE=InnoDB;

DROP TABLE IF EXISTS authors_books;
CREATE TABLE authors_books
(
	book_id INT,
	author_id INT
) ENGINE=InnoDB;

DROP TABLE IF EXISTS authors;
CREATE TABLE authors
(
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	PRIMARY KEY(id),
	name VARCHAR(255)
) ENGINE=InnoDB;

DROP TABLE IF EXISTS customers;
CREATE TABLE customers
(
	id INT UNSIGNED NOT NULL AUTO_INCREMENT,
	PRIMARY KEY(id),
	name VARCHAR(255)
) ENGINE=InnoDB;
