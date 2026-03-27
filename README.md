# Cuprite

Cuprite is an experimental web framework written in C, leveraging the high-performance `facil.io` event-driven I/O framework. The project aims to explore the feasibility of building a web development experience akin to Ruby on Rails, focusing on convention over configuration, rapid development, and a structured approach to web application creation, all within the performance characteristics of C.

## Usage

Quick start: `make quickstart` - this runs all the make targets bellow in order and starts the app on localhost:3001.
There are 2 demo apps: localhost:3001/products and localhost:3001/todos

To generate a new model, run the following command:

```bash
make generate_model model=<model_name>
```

This will look for `model_name.h` under `src/models` and generate the `h` and `c` files for the model in the `src/models/generated` directory, along with a migration file in `db/migrations`.

### Compile app

```bash
make
```

### Running Migrations

To apply any pending database migrations, run the following command after compiling the project:

```bash
make migrate
```

This will update your database schema to match the latest model definitions.

### Running the Application

To start the Cuprite web server, run the following command:

```bash
make start
```

The server will start and listen for requests on port 3001.
