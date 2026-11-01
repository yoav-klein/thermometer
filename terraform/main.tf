
terraform {
    required_providers {
        aws = {
            source  = "hashicorp/aws"
            version = "~> 6.0"
        }
    }
    backend "s3" {
        bucket = "terraform-state-yoav-thermostat"
        key = "state"
        region = "us-east-1"
    }
}

provider "aws" {
    region = "us-east-1"
}

data "aws_caller_identity" "current" {}

resource "aws_iot_thing" "thermostat" {
  name = "thermostat"
}

resource "aws_iot_certificate" "cert" {
  active = true
}

resource "aws_iot_policy" "thermostat" {
  name = "ThermostatConnectAndPublish"

  # Terraform's "jsonencode" function converts a
  # Terraform expression result to valid JSON syntax.
  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Action = [
          "iot:Connect"
        ]
        Effect   = "Allow"
        Resource = "arn:aws:iot:us-east-1:${data.aws_caller_identity.current.account_id}:client/thermostat"
      },
      {
        Action = [
            "iot:Publish"
        ]
        Effect = "Allow"
        Resource = "arn:aws:iot:us-east-1:${data.aws_caller_identity.current.account_id}:topic/measure/backbalcony"
      },
      {
        Action = [
            "iot:Subscribe",
        ]
        Effect = "Allow"
        Resource = "arn:aws:iot:us-east-1:${data.aws_caller_identity.current.account_id}:topicfilter/measure/backbalcony"
      },
      {
        Action = [
            "iot:Receive"
        ]
        Effect = "Allow"
        Resource = "arn:aws:iot:us-east-1:${data.aws_caller_identity.current.account_id}:topic/measure/backbalcony"
      }
    ]
  })
}

resource "aws_iot_policy_attachment" "att" {
  policy = aws_iot_policy.thermostat.name
  target = aws_iot_certificate.cert.arn
}

resource "local_sensitive_file" "privatekey" {
  content  = aws_iot_certificate.cert.private_key
  filename = "${path.module}/privatekey.pem"
}

resource "local_file" "certificate" {
  content  = aws_iot_certificate.cert.certificate_pem
  filename = "${path.module}/certificate.pem"
}
