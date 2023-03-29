-- this is a comment

CREATE TABLE table_name (
	column1 integer NOT NULL PRIMARY KEY,
	column2 varchar(20),
	column3 timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
	column4 smallint CHECK (column4 >= 0),
	CONSTRAINT ct1 CHECK (column4 IS NULL OR column2 IS NOT NULL)
);

INSERT INTO table_name (column1, column2, column3, ...)
VALUES (value1, value2, value3, ...);

BEGIN;
	UPDATE table_name
	SET balance = balance - 100.0
	WHERE account_id = '123';
-- more comment
COMMIT;

-- Oh no!
ROLLBACK;

-- Too late... We're $100 shorter...

CREATE TABLE other_table (
	col1 int4 NOT NULL PRIMARY KEY GENERATED BY DEFAULT AS IDENTITY,
	col2 integer REFERENCES table_name
);

-- Except first column, all others are fp numbers. (Dot should be included
-- in highlight). Note that last column is actually a sum of two floats,
-- so '+' should not be matched as part of exponential notation.
SELECT 1, 1., 1.1, .1, 1e, 1e2, 1.1e2, 12.12e-3, 7.e+4, 7.e+4.0;

-- 7e55 at the end is part of an identifier, not a floating point number.
-- (Whole identifier should be in one color).
CREATE DOMAIN no_realI7e55 AS integer CHECK (VALUE > 0);

-- CREATE OR REPLACE FUNCTION is one statement, so REPLACE should NOT be
-- highlighted as function name.
CREATE OR REPLACE FUNCTION function_name(IN param_name integer) RETURNS integer
LANGUAGE plpgsql AS $$
DECLARE
	ctr integer;
BEGIN
	-- 20..0 is a range, not two consecutive floats, so both dots should NOT
	-- be selected as part of any fp number and both numbers should be selected
	-- as integers.
	FOR ctr IN REVERSE 20..0 BY 2 LOOP
		RAISE NOTICE 'Counter: %', ctr;
	END LOOP;
END; $$

