# Cuprite

Cuprite is an experimental web framework written in C, leveraging the high-performance `facil.io` event-driven I/O framework. The project aims to explore the feasibility of building a web development experience akin to Ruby on Rails, focusing on convention over configuration, rapid development, and a structured approach to web application creation, all within the performance characteristics of C.

## Usage

To generate a new model, run the following command:

```bash
ruby scripts/generate_model.rb <model_name>
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
./bin/cuprite
```

The server will start and listen for requests on port 3001.
